#pragma once

#include "DNSServer.h"
#include "GxEPD2_BW.h"
#include "Preferences.h"
#include "qrcode.h"
#include "WebServer.h"
#include "Fonts/FreeSans12pt7b.h"
#include "Fonts/FreeSansBold12pt7b.h"

extern GxEPD2_BW<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT> display;
extern WebServer server;
extern DNSServer dnsServer;
extern Preferences preferences;

const auto access_point_ssid = "displayThingSetup";

inline void show_connected_screen()
{
    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeSansBold12pt7b);
        display.setTextSize(2);
        display.setCursor(50, 200);
        display.println("WiFi Connected!");
        display.setFont(&FreeSans12pt7b);
        display.setTextSize(1);
        display.setCursor(50, 250);
        display.print("IP: ");
        display.println(WiFi.localIP());
    }
    while (display.nextPage());
}

inline WebServer::THandlerFunction handle_save()
{
    return []()
    {
        const String ssid = server.arg("ssid");
        const String password = server.arg("password");

        if (ssid.length() > 0)
        {
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
        }
        else
        {
            server.send(400, "text/plain", "Bad Request: SSID cannot be empty.");
        }
    };
}

inline WebServer::THandlerFunction handle_root()
{
    return []()
    {
        String html = "<html><head><title>ESP32 WiFi Setup</title>";
        html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
        html += "<style>body{font-family: sans-serif; background: #f0f0f0; text-align: center;}";
        html += "div{background: white; padding: 20px; margin: 20px auto; max-width: 400px; border-radius: 8px;}";
        html += "select, input, button {width: 90%; padding: 12px; margin: 8px 0; border: 1px solid #ccc; border-radius: 4px;}";
        html += "button{background: #3498db; color: white; border: none; cursor: pointer;}</style></head>";
        html += "<body><div><h1>WiFi Setup</h1>";
        html += "<p>Please select a network and enter the password.</p>";
        html += "<form action='/save' method='POST'>";
        html += "<select id='ssid' name='ssid'>";

        Serial.println("Scanning for networks...");

        int networks = WiFi.scanNetworks();

        if (networks == 0)
        {
            Serial.println("No networks found...");
            html += "<option value=''>No networks found</option>";
        }
        else
        {
            for (int network = 0; network < networks; network++)
            {
                html += "<option value='" + WiFi.SSID(network) + "'>" + WiFi.SSID(network) + "(signal strength: " +
                    WiFi.RSSI(network) + ")" + "</option>";
            }
        }

        html += "</select>";
        html += "<input type='password' name='password' placeholder='Password'>";
        html += "<button type='submit'>Save & Connect</button>";
        html += "</form></div></body></html>";

        server.send(200, "text/html", html);
    };
}

inline void draw_qr_code(const esp_qrcode_handle_t qrcode)
{
    const int size = esp_qrcode_get_size(qrcode);

    if (size <= 0)
    {
        Serial.println("Could not get QR Code size");
        return;
    }

    constexpr int scale = 8;
    const int scaledSize = size * scale;
    const int offsetX = (display.width() - scaledSize) / 2;
    const int offsetY = (display.height() - scaledSize) / 2 + 60;

    for (int qr_y = 0; qr_y < size; qr_y++)
    {
        for (int qr_x = 0; qr_x < size; qr_x++)
        {
            if (esp_qrcode_get_module(qrcode, qr_x, qr_y))
            {
                display.fillRect(offsetX + qr_x * scale, offsetY + qr_y * scale, scale, scale, GxEPD_BLACK);
            }
        }
    }
}

inline void show_setup_screen(const char* ip)
{
    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeSansBold12pt7b);
        display.setTextSize(2);
        display.setCursor(50, 50);
        display.print("WiFi Setup");

        display.setFont(&FreeSans12pt7b);
        display.setTextSize(1);
        display.setCursor(50, 100);
        display.printf("1. Scan QR code or connect to %s.\n", access_point_ssid);
        display.setCursor(50, 130);
        display.printf("2. Add a network in your browser by accessing %s.\n", String("http://" + String(ip)).c_str());

        auto qrcode_config = ESP_QRCODE_CONFIG_DEFAULT();
        qrcode_config.display_func = &draw_qr_code;

        String network_payload = "WIFI:T:nopass;S:" + String(access_point_ssid) + ";;";

        const esp_err_t qrcode_error = esp_qrcode_generate(&qrcode_config, network_payload.c_str());

        if (qrcode_error != ESP_OK)
        {
            Serial.printf("Failed to generate QR code: %s", esp_err_to_name(qrcode_error));
        }
    }
    while (display.nextPage());

    Serial.println("Setup completed");
}

inline bool setup_wifi()
{
    WiFi.mode(WIFI_STA);

    preferences.begin("display-thing", false);
    const String ssid = preferences.getString("ssid", "");
    const String password = preferences.getString("password", "");

    if (ssid.length() > 0)
    {
        WiFi.begin(ssid.c_str(), password.c_str());
        Serial.print("WiFi Connecting");

        int connection_attempts = 0;
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
            connection_attempts++;
        }

        Serial.println();

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("WiFi connected.");
            preferences.end();
            show_connected_screen();
            return true;
        }

        Serial.println("Failed to connect to saved WiFi network.");
    }

    const auto esp32_ip = IPAddress(100, 100, 100, 100);

    WiFi.softAP(access_point_ssid);
    WiFi.softAPConfig(esp32_ip, esp32_ip, IPAddress(255, 255, 255, 0));
    dnsServer.start(53, "*", esp32_ip);

    server.on("/", handle_root());
    server.on("/save", HTTP_POST, handle_save());
    server.onNotFound(handle_root());
    server.begin();

    Serial.println("Configuration setup started at " + String(esp32_ip));
    show_setup_screen(esp32_ip.toString().c_str());

    return false;
}

inline void loop_wifi()
{
    dnsServer.processNextRequest();
    server.handleClient();
}
