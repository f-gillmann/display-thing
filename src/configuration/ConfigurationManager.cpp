#include "ConfigurationManager.h"

#include <WebServer.h>
#include <Preferences.h>
#include "web/configuration_portal.html.h"
#include "config.h"
#include "display/DisplayManager.h"

ConfigurationManager::ConfigurationManager(DisplayThing& displayThing) : displayThing(displayThing)
{
    loadConfiguration();
}

const DeviceConfig& ConfigurationManager::getConfig() const
{
    return m_config;
}

void ConfigurationManager::loadConfiguration()
{
    Preferences preferences;
    preferences.begin(PREFERENCES_DEVICE_CONFIG, true);

    // keys can't be too long since there is a length limit.
    m_config.interval = preferences.getUInt("interval", 30000);
    m_config.units = preferences.getString("units", "metric").c_str();
    m_config.apiKey = preferences.getString("om-api-key", "").c_str();

    preferences.end();
}


void ConfigurationManager::registerHandlers()
{
    auto& server = displayThing.getWebServer();
    auto& preferences = displayThing.getPreferences();

    server.on(
        "/", HTTP_GET, [&]()
        {
            server.sendHeader("Content-Encoding", "gzip");
            server.send_P(
                200, "text/html", reinterpret_cast<const char*>(CONFIGURATION_PORTAL_HTML_GZ),
                CONFIGURATION_PORTAL_HTML_GZ_LEN
            );
        }
    );

    server.on(
        "/save_config", HTTP_POST, [&]()
        {
            preferences.begin(PREFERENCES_DEVICE_CONFIG);

            preferences.putUInt("interval", server.arg("interval").toInt());
            preferences.putString("units", server.arg("units").c_str());
            preferences.putString("om-api-key", server.arg("om-api-key").c_str());

            preferences.end();
            loadConfiguration();

            const String response ="<html><head><style>body{background-color:#1e2030;color:#cad3f5;font-family:sans-serif;text-align:center;padding-top:50px;}a{color:#8aadf4;}</style></head><body><h1>Configuration Saved!</h1><p>Your settings have been updated.</p><br/><a href='/'>Go back</a></body></html>";
            server.send(200, "text/html", response);
        }
    );
}
