#include <Arduino.h>
#include "config.h"
#include "DisplayThing.h"
#include "display/configuration/ConfigurationManager.h"
#include "display/DisplayManager.h"
#include "display/screens/wifi_setup/WiFiSetupScreen.h"
#include "display/wifi/WiFiSetupManager.h"

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
unsigned long lastUpdate = 0;

void setup()
{
    displayThing = make_unique<DisplayThing>();
    wifiSetupManager = make_unique<WiFiSetupManager>(*displayThing);
    configManager = make_unique<ConfigurationManager>(*displayThing);
    displayManager = make_unique<DisplayManager>(*displayThing);

    configManager->onConfigChanged(
        [&](const DeviceConfig& newConfig)
        {
            Serial.println("Configuration updated. Rebuilding display queue.");
            displayManager->buildQueue(newConfig);
            lastUpdate = 0;
        }
    );

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
        displayManager->buildQueue(configManager->getConfig());
    }
    else
    {
        Serial.println("Not connected. Starting configuration portal.");
        std::string ap_password = wifiSetupManager->getAPPassword();
        std::string ap_ssid = wifiSetupManager->getAPSsid();
        const auto wifiScreen = make_unique<WiFiSetupScreen>(ap_ssid, ap_password);
        wifiScreen->show(*displayThing);
    }
}

void loop()
{
    if (isConnected)
    {
        // handle requests for config page
        displayThing->getWebServer().handleClient();

        const DeviceConfig& currentConfig = configManager->getConfig();
        const unsigned int current_duration = displayManager->getCurrentModuleDuration(currentConfig);

        if (millis() - lastUpdate > current_duration || lastUpdate == 0)
        {
            if (lastUpdate != 0)
            {
                displayManager->goToNextModule(currentConfig);
            }

            Serial.println("Updating and showing current module.");
            displayManager->updateCurrentModule();
            displayManager->showCurrentModule();

            lastUpdate = millis();
            displayThing->getDisplay().hibernate();
        }
    }
    else
    {
        // handle Wi-Fi captive portal
        wifiSetupManager->handleClient();
    }
}
