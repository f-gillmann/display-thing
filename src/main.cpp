#include <Arduino.h>
#include "config.h"
#include "DisplayThing.h"
#include "configuration/ConfigurationManager.h"
#include "display/DisplayManager.h"
#include "display/screens/info/InfoScreen.h"
#include "display/screens/wifi_setup/WiFiSetupScreen.h"
#include "wifi/WiFiSetupManager.h"

template <typename T, typename... Args>

std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

std::unique_ptr<DisplayThing> displayThing;
std::unique_ptr<WiFiSetupManager> wifiSetupManager;
std::unique_ptr<ConfigurationManager> configManager;
std::unique_ptr<DisplayManager> displayManager;

bool isConnected = false;
bool first_draw_done = false;
int interval = 30000;
unsigned long lastUpdate = 0;

void setup()
{
    // make these in setup because we have to wait until the nvs flash is initialized
    displayThing = make_unique<DisplayThing>();
    wifiSetupManager = make_unique<WiFiSetupManager>(*displayThing);
    configManager = make_unique<ConfigurationManager>(*displayThing);
    displayManager = make_unique<DisplayManager>(*displayThing);

    // add config change listener so we can reload the screen when it changed
    configManager->onConfigChanged(
        [&](const DeviceConfig& newConfig)
        {
            Serial.printf("Configuration updated. New interval: %d. Forcing display refresh.\n", newConfig.interval);
            interval = static_cast<int32_t>(newConfig.interval);
            displayManager->update(newConfig);
        }
    );

    pinMode(EPD_PWR, OUTPUT);
    digitalWrite(EPD_PWR, HIGH);

    Serial.begin(115200);

    auto& display = displayThing->getDisplay();
    SPI.begin(EPD_SCK, EPD_MISO, EPD_MOSI);
    display.init(115200, true, 2, false);
    display.setRotation(2);

    isConnected = wifiSetupManager->connect();

    if (isConnected)
    {
        Serial.printf("Connected. IP: %s\n", WiFi.localIP().toString().c_str());

        // register configuration website handlers and start webserver
        configManager->registerHandlers();
        displayThing->getWebServer().begin();

        // load config from nvs flash and set configured interval
        const DeviceConfig& currentConfig = configManager->getConfig();
        interval = static_cast<int32_t>(currentConfig.interval);

        displayManager->setScreen(make_unique<InfoScreen>());
    }
    else
    {
        Serial.println("Not connected. Starting configuration portal.");
        std::string ap_password = wifiSetupManager->getAPPassword();
        std::string ap_ssid = wifiSetupManager->getAPSsid();
        displayManager->setScreen(make_unique<WiFiSetupScreen>(ap_ssid, ap_password));
    }
}

void loop()
{
    if (isConnected)
    {
        // handle requests for config page
        displayThing->getWebServer().handleClient();

        if (!first_draw_done || (millis() - lastUpdate > interval))
        {
            const DeviceConfig& currentConfig = configManager->getConfig();

            displayManager->update(currentConfig);
            lastUpdate = millis();
            first_draw_done = true;

            displayThing->getDisplay().hibernate();
        }
    }
    else
    {
        // handle Wi-Fi captive portal
        wifiSetupManager->handleClient();
    }
}
