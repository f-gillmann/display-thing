#include <Arduino.h>
#include "DisplayThing.h"
#include "config.h"
#include "configuration/ConfigurationManager.h"
#include "wifi/WiFiSetupManager.h"
#include "display/DisplayManager.h"
#include "display/screens/info/InfoScreen.h"
#include "display/screens/wifi_setup/WiFiSetupScreen.h"

template<typename T, typename... Args>

std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

DisplayThing displayThing;
WiFiSetupManager wifiSetupManager(displayThing);
ConfigurationManager configManager(displayThing);
DisplayManager displayManager(displayThing);

bool isConnected = false;
bool first_draw_done = false;
int updateInterval = 30000;
unsigned long lastUpdate = 0;

void setup()
{
    pinMode(EPD_PWR, OUTPUT);
    digitalWrite(EPD_PWR, HIGH);

    Serial.begin(115200);

    auto& display = displayThing.getDisplay();
    SPI.begin(EPD_SCK, EPD_MISO, EPD_MOSI);
    display.init(115200, true, 2, false);
    display.setRotation(2);

    isConnected = wifiSetupManager.connect();

    if (isConnected)
    {
        Serial.printf("Connected. IP: %s\n", WiFi.localIP().toString().c_str());

        // load config from flash and set configured interval
        const DeviceConfig& currentConfig = configManager.getConfig();
        updateInterval = static_cast<int32_t>(currentConfig.interval);

        configManager.registerHandlers();
        displayThing.getWebServer().begin();

        displayManager.setScreen(make_unique<InfoScreen>(currentConfig));
    }
    else
    {
        Serial.println("Not connected. Starting configuration portal.");
        std::string ap_password = wifiSetupManager.getAPPassword();
        displayManager.setScreen(make_unique<WiFiSetupScreen>(ap_password));
    }
}

void loop()
{
    if (isConnected)
    {
        // handle requests for config page
        displayThing.getWebServer().handleClient();

        if (!first_draw_done || (millis() - lastUpdate > updateInterval))
        {
            displayManager.update();
            lastUpdate = millis();
            first_draw_done = true;
        }
    }
    else
    {
        // handle wifi captive portal
        wifiSetupManager.handleClient();
    }
}
