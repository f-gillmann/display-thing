#include <Arduino.h>
#include "config.h"
#include "globals.h"
#include "screens/temp/TempScreen.h"
#include "wifi_config/WifiConfigManager.h"

bool isConnected = false;
bool first_draw_done = false;
constexpr long updateInterval = 30000;
unsigned long lastUpdate = 0;

void setup()
{
    pinMode(EPD_PWR, OUTPUT);
    digitalWrite(EPD_PWR, HIGH);

    Serial.begin(115200);
    Serial.println("Setup starting...");

    SPI.begin(EPD_SCK, EPD_MISO, EPD_MOSI);
    display.init(115200, true, 2, false);
    display.setRotation(2);

    isConnected = setup_wifi();

    if (isConnected)
    {
        Serial.println("Connected.");
        Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
    }
    else
    {
        Serial.println("Not connected, configure in your browser.");
    }

    Serial.println("Setup finished.");
}

void loop()
{
    if (isConnected)
    {
        if (!first_draw_done || (millis() - lastUpdate > updateInterval))
        {
            show_temp_screen();
            lastUpdate = millis();
            first_draw_done = true;
        }
    }
    else
    {
        loop_wifi();
    }
}
