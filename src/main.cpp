#include <Arduino.h>
#include <nvs_flash.h>
#include "config.h"
#include "DisplayThing.h"
#include "Logger.hpp"
#include "util.hpp"
#include "display/configuration/ConfigurationManager.h"
#include "display/DisplayManager.h"
#include "display/time/TimeManager.h"
#include "display/wifi/WiFiSetupManager.h"

std::unique_ptr<DisplayThing> displayThing;
std::unique_ptr<WiFiSetupManager> wifiSetupManager;
std::unique_ptr<ConfigurationManager> configManager;
std::unique_ptr<DisplayManager> displayManager;
std::unique_ptr<TimeManager> timeManager;

bool isConnected = false;
unsigned long lastUpdate = 0;
int lastMinute = -1;

WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP, "pool.ntp.org");

#ifndef DISABLE_DEBUG
LoggerClass Logger;
#endif

void setup()
{
    Serial.begin(115200);

#ifndef DISABLE_DEBUG
    Logger.begin(Serial, LogLevel::DEBUG);
#endif

    // Initialize NVS flash for Preferences
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        LOG_WARN("NVS flash init failed, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    displayThing = make_unique<DisplayThing>();
    wifiSetupManager = make_unique<WiFiSetupManager>(*displayThing);
    configManager = make_unique<ConfigurationManager>(*displayThing);
    timeManager = make_unique<TimeManager>(ntpClient, *configManager);
    displayManager = make_unique<DisplayManager>(*displayThing, *timeManager);

    // listener for config changes, update the screen and
    // log new config as soon as the config updates
    configManager->onConfigChanged(
        [&](const DeviceConfig& newConfig)
        {
            LOG_INFO("Configuration updated. Forcing immediate redraw.");
            configManager->logConfiguration();

            displayManager->buildQueue(newConfig);

            displayManager->updateCurrentModule();
            displayManager->showCurrentModule();

            lastUpdate = millis();
            displayThing->getDisplay().hibernate();
        }
    );

    // log current config
    configManager->logConfiguration();

    auto& display = displayThing->getDisplay();
    SPI.begin(EPD_SCK, EPD_MISO, EPD_MOSI);
    display.init(115200, true, 2, false);
    display.setRotation(2);

    isConnected = wifiSetupManager->connect();

    if (isConnected)
    {
        LOG_INFO("Connected. IP: %s", WiFi.localIP().toString().c_str());

        // register configuration website handlers and start webserver
        configManager->registerHandlers();
        displayThing->getWebServer().begin();

        // start time manager
        timeManager->begin();

        // build the module queue from our saved configuration
        displayManager->buildQueue(configManager->getConfig());
    }
}

void loop()
{
    if (WiFiClass::status() == WL_CONNECTED)
    {
        if (!isConnected)
        {
            LOG_INFO("WiFi connection re-established. Restarting network services.");
            isConnected = true;

            // re-initialize network services and time manager
            timeManager->begin();
            displayThing->getWebServer().begin();

            // force an immediate screen update on reconnect
            lastUpdate = 0;
            lastMinute = -1;
        }

        // handle requests for config page
        displayThing->getWebServer().handleClient();

        // update time
        ntpClient.update();

        const DeviceConfig& currentConfig = configManager->getConfig();
        const unsigned int current_duration = displayManager->getCurrentModuleDuration(currentConfig);
        const unsigned long currentMillis = millis();
        const int currentMinute = ntpClient.getMinutes();

        // update current module if the current modules duration has been reached
        // or if we forced a refresh by setting lastUpdate to 0
        if (currentMillis - lastUpdate > current_duration || lastUpdate == 0)
        {
            // show next module
            if (lastUpdate != 0)
            {
                displayManager->goToNextModule(currentConfig);
            }

            displayManager->updateCurrentModule();
            displayManager->showCurrentModule();

            lastUpdate = currentMillis;
            displayThing->getDisplay().hibernate();
        }
        // update current module, only do it if the module duration is above a minute
        else if (currentMinute != lastMinute && current_duration > 60000)
        {
            displayManager->updateCurrentModule();
            displayManager->showCurrentModule();

            lastMinute = currentMinute;
            displayThing->getDisplay().hibernate();
        }
    }
    else
    {
        if (isConnected)
        {
            LOG_WARN("WiFi connection lost. Stopping network services.");
            isConnected = false;

            // stop all network services
            timeManager->end();
            displayThing->getWebServer().stop();
        }

        wifiSetupManager->manageConnection();
    }
}
