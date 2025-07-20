#include "WifiConfigManager.h"

#include <WiFi.h>
#include "WifiConfigDisplay.h"
#include "WifiConfigPortal.h"
#include "../globals.h"

static void handle_save()
{
    const String ssid = server.arg("ssid");
    const String password = server.arg("password");

    if (ssid.length() > 0) {
        Serial.println("Saving WiFi credentials...");
        preferences.putString("ssid", ssid);
        preferences.putString("password", password);

        String response = "<html><body style='font-family: sans-serif; text-align: center;'>";
        response += "<h1>Credentials Saved!</h1>";
        response += "<p>The ESP32 will now restart and try to connect to '<strong>" + ssid + "</strong>'.</p>";
        response += "</body></html>";
        server.send(200, "text/html", response);

        delay(2000);
        ESP.restart();
    } else {
        server.send(400, "text/plain", "Bad Request: SSID cannot be empty.");
    }
}

static void handle_scan()
{
    Serial.println("Scanning for networks (API request)...");
    const int network_results = WiFi.scanNetworks();
    Serial.println("Scan complete.");

    String json = "[";
    if (network_results > 0) {
        for (int i = 0; i < network_results; ++i) {
            if (i > 0) {
                json += ",";
            }
            json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
        }
    }
    json += "]";

    WiFi.scanDelete();
    server.send(200, "application/json", json);
}

static void handle_root()
{
    server.send_P(200, "text/html", WIFI_CONFIG_PORTAL);
}


bool setup_wifi()
{
    WiFi.mode(WIFI_STA);

    // get saved ssid and password
    preferences.begin("display_thing", false);
    const String ssid = preferences.getString("ssid", "");
    const String password = preferences.getString("password", "");

    // if we have a ssid/password saved try to connect
    if (ssid.length() > 0) {
        WiFi.begin(ssid.c_str(), password.c_str());
        Serial.print("WiFi Connecting");

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) { // 10 second timeout
            delay(500);
            Serial.print(".");
            attempts++;
        }

        Serial.println();

        // return true if we have established a connection
        // else we will break out of the statement and proceed to set up a network
        if (WiFi.status() == WL_CONNECTED) {
            preferences.end();
            return true;
        }

        Serial.println("Failed to connect to saved WiFi. Starting AP.");
    }

    const auto access_point_password = String(random(10000000, 100000000));
    const auto esp32_ip = IPAddress(100, 100, 100, 100);

    // create ap for setting up a connection
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ACCESS_POINT_SSID, access_point_password.c_str(), 1, false, 1);
    WiFi.softAPConfig(esp32_ip, esp32_ip, IPAddress(255, 255, 255, 0));
    dnsServer.start(53, "*", esp32_ip);

    // handle routes
    server.on("/", HTTP_GET, handle_root);
    server.on("/scan", HTTP_GET, handle_scan);
    server.on("/save", HTTP_POST, handle_save);
    server.on("/favicon.ico", []() { server.send(204); });
    server.onNotFound(handle_root);
    server.begin();

    // display screen
    Serial.println("Configuration setup started at http://" + esp32_ip.toString());
    show_setup_screen(esp32_ip.toString().c_str(), access_point_password.c_str());

    return false;
}

void loop_wifi()
{
    dnsServer.processNextRequest();
    server.handleClient();
}
