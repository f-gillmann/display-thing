#include "WiFiSetupManager.h"
#include <WiFi.h>
#include "display/web/wifi_setup_portal.html.h"
#include <vector>
#include <algorithm>
#include "Logger.hpp"
#include "util.hpp"
#include "display/screens/wifi_reconnect/WiFiReconnectScreen.h"
#include "display/screens/wifi_setup/WiFiSetupScreen.h"

static std::string generateRandomSSID()
{
    static const std::vector<std::string> words = {
        "loop", "wave", "garden", "river", "bridge", "maple", "temple", "engine", "spire",
        "matrix", "node", "stream", "branch", "portal", "frame", "shell", "thread", "core",
        "bonsai", "koi", "petal", "fern", "cluster", "pixel", "leaf", "shard", "ember"
    };

    const int word1 = random(0, static_cast<int>(words.size()));
    const int word2 = random(0, static_cast<int>(words.size()));

    std::string ssid = words[word1] + "-" + words[word2];
    std::transform(ssid.begin(), ssid.end(), ssid.begin(), ::tolower);

    return ssid;
}

WiFiSetupManager::WiFiSetupManager(DisplayThing& displayThing) : displayThing(displayThing)
{
}

std::string WiFiSetupManager::getAPPassword() const
{
    return access_point_password;
}

std::string WiFiSetupManager::getAPSsid() const
{
    return access_point_ssid;
}

void WiFiSetupManager::startAP()
{
    access_point_password = String(random(10000000, 100000000)).c_str();
    access_point_ssid = generateRandomSSID();

    LOG_INFO("AP SSID: %s", access_point_ssid.c_str());
    LOG_INFO("AP Password: %s", access_point_password.c_str());

    WiFiClass::mode(WIFI_AP);
    WiFi.softAP(access_point_ssid.c_str(), access_point_password.c_str(), 1, false, 1);
    WiFi.softAPConfig(ACCESS_POINT_IP, ACCESS_POINT_IP, IPAddress(255, 255, 255, 0));

    auto& preferences = displayThing.getPreferences();
    auto& dnsServer = displayThing.getDnsServer();
    auto& server = displayThing.getWebServer();

    dnsServer.start(53, "*", ACCESS_POINT_IP);

    server.on(
        "/", HTTP_GET, [&]()
        {
            server.sendHeader("Content-Encoding", "gzip");
            server.send_P(
                200, "text/html", reinterpret_cast<const char*>(WIFI_SETUP_PORTAL_HTML_GZ),
                WIFI_SETUP_PORTAL_HTML_GZ_LEN
            );
        }
    );

    server.on(
        "/scan", HTTP_GET, [&]()
        {
            const int networks = WiFi.scanNetworks();
            String json = "[";

            if (networks > 0)
            {
                for (int network = 0; network < networks; ++network)
                {
                    if (network > 0) json += ",";
                    json += R"({"ssid":")" + WiFi.SSID(network) + R"(","rssi":)" + String(WiFi.RSSI(network)) + "}";
                }
            }

            json += "]";

            WiFi.scanDelete();
            server.send(200, "application/json", json);
        }
    );

    server.on(
        "/save", HTTP_POST, [&]()
        {
            if (!preferences.begin(PREFERENCES_WIFI_CONFIG))
            {
                server.send(500, "application/json", R"({"error":"Could not open storage"})");
                return;
            }

            if (server.hasArg("ssid") && server.hasArg("password"))
            {
                const String ssid = server.arg("ssid").c_str();
                const String password = server.arg("password").c_str();

                if (ssid.length() > 0)
                {
                    preferences.putString("ssid", ssid);
                    preferences.putString("password", password);
                    preferences.end();

                    server.send(
                        200, "application/json", R"({"status":"success", "message":"Credentials saved. Restarting..."})"
                    );

                    delay(500);
                    ESP.restart();
                }
                else
                {
                    server.send(400, "application/json", R"({"status":"error", "message":"SSID cannot be empty"})");
                }
            }
            else
            {
                server.send(
                    400, "application/json", R"({"status":"error", "message":"Missing ssid or password parameters"})"
                );
            }

            preferences.end();
        }
    );

    server.on(
        "/favicon.ico", [&]()
        {
            server.send(204);
        }
    );

    server.onNotFound(
        [&]()
        {
            server.sendHeader("Content-Encoding", "gzip");
            server.send_P(
                200, "text/html", reinterpret_cast<const char*>(WIFI_SETUP_PORTAL_HTML_GZ),
                WIFI_SETUP_PORTAL_HTML_GZ_LEN
            );
        }
    );

    server.begin();
}

bool WiFiSetupManager::attemptConnection() const
{
    WiFiClass::mode(WIFI_STA);
    auto& preferences = displayThing.getPreferences();
    preferences.begin(PREFERENCES_WIFI_CONFIG, true);

    const String ssid = preferences.getString("ssid", "");
    const String password = preferences.getString("password", "");

    preferences.end();

    if (ssid.length() > 0)
    {
        WiFi.begin(ssid.c_str(), password.c_str());

        int attempts = 0;
        while (WiFiClass::status() != WL_CONNECTED && attempts < 20)
        {
            delay(500);
            attempts++;
        }

        return WiFiClass::status() == WL_CONNECTED;
    }

    return false;
}

bool WiFiSetupManager::connect()
{
    auto& preferences = displayThing.getPreferences();
    preferences.begin(PREFERENCES_WIFI_CONFIG, true);
    const bool hasCredentials = preferences.getString("ssid", "").length() > 0;
    preferences.end();

    if (attemptConnection())
    {
        return true;
    }

    if (!hasCredentials) {
        std::string ap_ssid = getAPSsid();
        std::string ap_password = getAPPassword();

        const auto setupScreen = make_unique<WiFiSetupScreen>(ap_ssid, ap_password);
        setupScreen->show(displayThing);
    }

    startAP();

    return false;
}

void WiFiSetupManager::handleClient() const
{
    if (WiFi.getMode() == WIFI_AP)
    {
        displayThing.getDnsServer().processNextRequest();
        displayThing.getWebServer().handleClient();
    }
}

bool WiFiSetupManager::manageConnection()
{
    auto& preferences = displayThing.getPreferences();
    preferences.begin(PREFERENCES_WIFI_CONFIG, true);
    const bool hasCredentials = preferences.getString("ssid", "").length() > 0;
    preferences.end();

    // start AP if needed
    if ((!hasCredentials || reconnectAttempts >= maxReconnectAttempts) && WiFi.getMode() != WIFI_AP)
    {
        LOG_INFO("Starting configuration portal");
        startAP();

        std::string ap_ssid = getAPSsid();
        std::string ap_password = getAPPassword();

        const auto reconnectScreen = make_unique<WiFiReconnectScreen>(ap_ssid, ap_password);
        reconnectScreen->show(displayThing);
    }

    handleClient();

    // try reconnection if we have credentials
    if (hasCredentials && WiFi.getMode() != WIFI_AP)
    {
        const unsigned long currentMillis = millis();
        if (currentMillis - lastReconnectAttempt > reconnectInterval)
        {
            LOG_INFO("Attempting to connect to WiFi (Attempt %d)...", reconnectAttempts + 1);
            lastReconnectAttempt = currentMillis;

            if (attemptConnection())
            {
                LOG_INFO("Connected successfully! IP: %s", WiFi.localIP().toString().c_str());
                reconnectAttempts = 0;

                return true;
            }

            LOG_WARN("Connection attempt failed.");
            reconnectAttempts++;
        }
    }

    return false;
}
