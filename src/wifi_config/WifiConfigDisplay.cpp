#include "WifiConfigDisplay.h"

#include <qrcode.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include "../globals.h"

static void draw_qr_code(const esp_qrcode_handle_t qrcode)
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

void show_setup_screen(const char* esp32_ip, const char* access_point_password)
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

        String network_payload = "WIFI:T:WPA;S:" + String(ACCESS_POINT_SSID) + ";P:" + String(access_point_password) +
            ";;";
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
        display.print(ACCESS_POINT_SSID);

        wifi_credentials_y += 35;
        display.setFont(&FreeSans9pt7b);
        display.setCursor(wifi_credentials_x, wifi_credentials_y);
        display.print("Password: ");
        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(wifi_credentials_x + 100, wifi_credentials_y);
        display.print(access_point_password);
    }
    while (display.nextPage());

    Serial.println("Setup screen displayed.");
}
