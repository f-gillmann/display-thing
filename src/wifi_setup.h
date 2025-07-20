#pragma once

#include "DNSServer.h"
#include "GxEPD2_BW.h"
#include "Preferences.h"
#include "qrcode.h"
#include "WebServer.h"
#include "Fonts/FreeSans12pt7b.h"
#include "Fonts/FreeSansBold12pt7b.h"
#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSansBold9pt7b.h"

extern GxEPD2_BW<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT> display;
extern WebServer server;
extern DNSServer dnsServer;
extern Preferences preferences;

const auto access_point_ssid = "display_thing";

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

inline WebServer::THandlerFunction handle_scan()
{
    return []()
    {
        Serial.println("Scanning for networks (API request)...");
        const int network_results = WiFi.scanNetworks();
        Serial.println("Scan complete.");

        String json = "[";
        if (network_results > 0)
        {
            for (int i = 0; i < network_results; ++i)
            {
                if (i > 0)
                {
                    json += ",";
                }
                json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
            }
        }
        json += "]";

        WiFi.scanDelete();

        server.send(200, "application/json", json);
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
        html += "<select id='ssid' name='ssid'><option value=''>Scanning...</option></select>";
        html += "<input type='password' name='password' placeholder='Password'>";
        html += "<button type='submit'>Save & Connect</button>";
        html += "</form>";
        html += "<script>";
        html += "window.addEventListener('load', function() {";
        html += "  fetch('/scan')";
        html += "    .then(response => response.json())";
        html += "    .then(data => {";
        html += "      var select = document.getElementById('ssid');";
        html += "      select.innerHTML = '';";
        html += "      if (data.length === 0) {";
        html += "        select.innerHTML = '<option value=\"\">No networks found</option>';";
        html += "      } else {";
        html += "        data.sort((a, b) => b.rssi - a.rssi);";
        html += "        data.forEach(net => {";
        html += "          var opt = document.createElement('option');";
        html += "          opt.value = net.ssid;";
        html += "          opt.textContent = `${net.ssid} (${net.rssi} dBm)`;";
        html += "          select.appendChild(opt);";
        html += "        });";
        html += "      }";
        html += "    })";
        html += "    .catch(error => {";
        html += "      console.error('Error fetching networks:', error);";
        html += "      document.getElementById('ssid').innerHTML = '<option value=\"\">Scan failed</option>';";
        html += "    });";
        html += "});";
        html += "</script>";
        html += "</div></body></html>";

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

    constexpr int scale = 10;
    const int scaledSize = size * scale;
    const int offsetX = (display.width() - scaledSize) / 2 + display.width() / 4;
    const int offsetY = (display.height() - scaledSize) / 2 - 25;

    for (int qr_y = 0; qr_y < size; qr_y++)
    {
        for (int qr_x = 0; qr_x < size; qr_x++)
        {
            if (esp_qrcode_get_module(qrcode, qr_x, qr_y))
            {
                display.fillRect(
                    static_cast<int16_t>(offsetX + qr_x * scale), static_cast<int16_t>(offsetY + qr_y * scale),
                    scale, scale, GxEPD_BLACK
                );
            }
        }
    }
}

inline void show_setup_screen(const char* esp32_ip, const char* access_point_password)
{
    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);

        // layout bounds
        const int center_x = display.width() / 2;
        constexpr int margin = 30;

        // --- center divider
        display.drawFastVLine(static_cast<int16_t>(center_x), 0, display.height(), GxEPD_BLACK);

        // --- left side
        display.setTextColor(GxEPD_BLACK);

        // title
        constexpr auto title = "WiFi Setup";
        display.setFont(&FreeSansBold12pt7b);
        display.setTextSize(2);
        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
        display.setCursor(static_cast<int16_t>((center_x - w) / 2), 80);
        display.print(title);

        // setup instructions on the left side
        display.setFont(&FreeSans12pt7b);
        display.setTextSize(1);

        display.setCursor(margin, 180);
        display.print("1. Use the QR code or details on");
        display.setCursor(margin + 25, 205);
        display.print("the right to connect to WiFi.");

        display.setCursor(margin, 280);
        display.print("2. A 'Setup' page should open.");

        display.setCursor(margin + 25, 310);
        display.print("If not, open this url:");

        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(margin + 25, 345);
        display.print("http://");
        display.print(esp32_ip);

        // --- right side
        auto qrcode_config = ESP_QRCODE_CONFIG_DEFAULT();
        qrcode_config.display_func = &draw_qr_code;

        String network_payload = "WIFI:T:WPA;S:" + String(access_point_ssid) + ";P:" + String(access_point_password) + ";;";
        const esp_err_t qrcode_error = esp_qrcode_generate(&qrcode_config, network_payload.c_str());

        if (qrcode_error != ESP_OK)
        {
            Serial.printf("Failed to generate QR code: %s", esp_err_to_name(qrcode_error));
        }

        // credentials for access point
        const int16_t wifi_credentials_x = center_x + 75;
        int16_t wifi_credentials_y = display.height() - 75;

        display.setFont(&FreeSans9pt7b);
        display.setCursor(wifi_credentials_x, wifi_credentials_y);
        display.print("Network: ");
        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(wifi_credentials_x + 100, wifi_credentials_y);
        display.print(access_point_ssid);

        wifi_credentials_y += 35;
        display.setFont(&FreeSans9pt7b);
        display.setCursor(wifi_credentials_x, wifi_credentials_y);
        display.print("Password: ");
        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(wifi_credentials_x + 100, wifi_credentials_y);
        display.print(access_point_password);
    }
    while (display.nextPage());

    Serial.println("Setup completed");
}

inline bool setup_wifi()
{
    WiFi.mode(WIFI_MODE_STA);

    preferences.begin("display_thing", false);
    const String ssid = preferences.getString("ssid", "");
    const String password = preferences.getString("password", "");

    if (ssid.length() > 0)
    {
        WiFi.begin(ssid.c_str(), password.c_str());
        Serial.print("WiFi Connecting");

        int connection_attempts = 0;
        while (WiFiClass::status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
            connection_attempts++;
        }

        Serial.println();

        if (WiFiClass::status() == WL_CONNECTED)
        {
            Serial.println("WiFi connected.");
            preferences.end();
            show_connected_screen();
            return true;
        }

        Serial.println("Failed to connect to saved WiFi network.");
    }

    const auto access_point_password = String(random(10000000, 100000000)); // random 8 digit password
    const auto esp32_ip = IPAddress(100, 100, 100, 100);

    WiFi.mode(WIFI_MODE_AP);
    WiFi.softAP(access_point_ssid, access_point_password, 1, false, 1);
    WiFi.softAPConfig(esp32_ip, esp32_ip, IPAddress(255, 255, 255, 0));
    dnsServer.start(53, "*", esp32_ip);

    server.on("/", handle_root());
    server.on("/scan", handle_scan());
    server.on("/save", HTTP_POST, handle_save());
    server.on("/favicon.ico", []() { server.send(204); });
    server.onNotFound(handle_root());
    server.begin();

    Serial.println("Configuration setup started at " + String(esp32_ip));
    show_setup_screen(esp32_ip.toString().c_str(), access_point_password.c_str());

    return false;
}

inline void loop_wifi()
{
    dnsServer.processNextRequest();
    server.handleClient();
}
