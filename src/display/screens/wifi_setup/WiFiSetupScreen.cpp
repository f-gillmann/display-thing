#include "WiFiSetupScreen.h"

#include <qrcode.h>
#include <utility>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include "display/DisplayManager.h"

static GxEPD2_BW<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT>* g_current_display = nullptr;

static void draw_qr_code(const esp_qrcode_handle_t qrcode)
{
    if (!g_current_display)
    {
        return;
    }

    auto& display = *g_current_display;
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

    for (int y = 0; y < size; y++)
    {
        for (int x = 0; x < size; x++)
        {
            if (esp_qrcode_get_module(qrcode, x, y))
            {
                display.fillRect(
                    static_cast<int16_t>(offsetX + x * scale),
                    static_cast<int16_t>(offsetY + y * scale),
                    scale,
                    scale,
                    GxEPD_BLACK
                );
            }
        }
    }
}

WiFiSetupScreen::WiFiSetupScreen(std::string ap_ssid, std::string ap_password)
    : access_point_ssid(std::move(ap_ssid)), access_point_password(std::move(ap_password))
{
}

void WiFiSetupScreen::setConfig(const DeviceConfig& deviceConfig)
{
}


void WiFiSetupScreen::show(DisplayThing& displayThing)
{
    auto& display = displayThing.getDisplay();

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
        display.print(ACCESS_POINT_IP);

        // --- right side
        auto qrcode_config = ESP_QRCODE_CONFIG_DEFAULT();
        qrcode_config.display_func = &draw_qr_code;

        // set current display in a global variable so we can access it in our draw_qr_code function
        g_current_display = &display;

        String network_payload = "WIFI:T:WPA;S:" + String(access_point_ssid.c_str()) + ";P:" + access_point_password.c_str() + ";;";
        const esp_err_t qrcode_error = esp_qrcode_generate(&qrcode_config, network_payload.c_str());

        // set the current display back to the nullptr after use
        g_current_display = nullptr;

        if (qrcode_error != ESP_OK)
        {
            Serial.printf("Failed to generate QR code: %s", esp_err_to_name(qrcode_error));
        }

        // credentials for access point
        const auto wifi_credentials_x = static_cast<int16_t>(center_x + 75);
        auto wifi_credentials_y = static_cast<int16_t>(display.height() - 75);

        display.setFont(&FreeSans9pt7b);
        display.setCursor(wifi_credentials_x, wifi_credentials_y);
        display.print("Network: ");
        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(static_cast<int16_t>(wifi_credentials_x + 100), wifi_credentials_y);
        display.print(access_point_ssid.c_str());

        wifi_credentials_y += 35;
        display.setFont(&FreeSans9pt7b);
        display.setCursor(wifi_credentials_x, wifi_credentials_y);
        display.print("Password: ");
        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(static_cast<int16_t>(wifi_credentials_x + 100), wifi_credentials_y);
        display.print(access_point_password.c_str());
    }
    while (display.nextPage());

    Serial.println("Setup screen displayed.");
}
