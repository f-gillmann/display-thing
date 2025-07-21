#include "WiFiSetupManager.h"

#include <WiFi.h>
#include "web/wifi_setup_portal.html.h"

WiFiSetupManager::WiFiSetupManager(DisplayThing& displayThing) : displayThing(displayThing) { }

std::string WiFiSetupManager::getAPPassword() const
{
    return ap_password;
}

void WiFiSetupManager::startAP()
{
    ap_password = String(random(10000000, 100000000)).c_str();

    WiFiClass::mode(WIFI_AP);
    WiFi.softAP(ACCESS_POINT_SETUP_SSID, ap_password.c_str(), 1, false, 1);
    WiFi.softAPConfig(ACCESS_POINT_IP, ACCESS_POINT_IP, IPAddress(255, 255, 255, 0));

    auto& preferences = displayThing.getPreferences();
    auto& dnsServer = displayThing.getDnsServer();
    auto& server = displayThing.getWebServer();

    dnsServer.start(53, "*", ACCESS_POINT_IP);
    preferences.begin(PREFERENCES_WIFI_CONFIG);

    server.on("/", HTTP_GET, [&]() {
        server.sendHeader("Content-Encoding", "gzip");
        server.send_P(200, "text/html", reinterpret_cast<const char*>(WIFI_SETUP_PORTAL_HTML_GZ), WIFI_SETUP_PORTAL_HTML_GZ_LEN);
    });

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
            const String ssid = server.arg("ssid");

            if (ssid.length() > 0)
            {
                preferences.putString("ssid", ssid);
                preferences.putString("password", server.arg("password"));
                preferences.end();

                const String response = "<html><head><style>body{background-color:#1e2030;color:#cad3f5;font-family:sans-serif;text-align:center;padding-top:50px;}</style></head><body><h1>Credentials Saved!</h1><p>The device will now restart to connect to '" + ssid + "'.</p></body></html>";
                server.send(200, "text/html", response);

                delay(2000);
                ESP.restart();
            }
            else
            {
                server.send(400, "text/plain", "Bad Request");
            }
        }
    );

    // send not found to prevent warnings in serial
    server.on(
        "/favicon.ico", [&]()
        {
            server.send(204);
        }
    );

    server.onNotFound([&]() {
        server.sendHeader("Content-Encoding", "gzip");
        server.send_P(200, "text/html", reinterpret_cast<const char*>(WIFI_SETUP_PORTAL_HTML_GZ), WIFI_SETUP_PORTAL_HTML_GZ_LEN);
    });

    server.begin();
}


bool WiFiSetupManager::connect()
{
    WiFiClass::mode(WIFI_STA);
    auto& preferences = displayThing.getPreferences();
    preferences.begin(PREFERENCES_WIFI_CONFIG, false);

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

        if (WiFiClass::status() == WL_CONNECTED)
        {
            return true;
        }
    }

    startAP();
    return false;
}

void WiFiSetupManager::handleClient() const
{
    displayThing.getDnsServer().processNextRequest();
    displayThing.getWebServer().handleClient();
}
