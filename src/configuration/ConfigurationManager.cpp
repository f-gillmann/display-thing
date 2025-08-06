#include "ConfigurationManager.h"

#include <Preferences.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "config.h"
#include "display/DisplayManager.h"
#include "web/configuration_portal.html.h"

ConfigurationManager::ConfigurationManager(DisplayThing& displayThing) : displayThing(displayThing)
{
    loadConfiguration();
}

void ConfigurationManager::onConfigChanged(const ConfigChangeCallback& callback)
{
    m_onConfigChangeCallback = callback;
}

const DeviceConfig& ConfigurationManager::getConfig() const
{
    return m_config;
}

void ConfigurationManager::loadConfiguration()
{
    Preferences preferences;
    preferences.begin(PREFERENCES_DEVICE_CONFIG);
    // keys can't be too long since there is a length limit.

    // load general settings
    m_config.interval = preferences.getUInt("interval", 30000);
    m_config.units = preferences.getString("units", "metric").c_str();

    // load weather settings
    m_config.weather_lat = preferences.getFloat("w_lat", 50.77f); // Default to Haiger
    m_config.weather_lon = preferences.getFloat("w_lon", 8.20f);
    m_config.weather_service = preferences.getString("w_svc", "openmeteo");
    m_config.weather_apikey = preferences.getString("w_key", "");

    m_config.queue.clear();
    const int queueSize = preferences.getInt("q_size", 0);
    for (int i = 0; i < queueSize; ++i)
    {
        char nameKey[10];
        char durKey[10];
        snprintf(nameKey, sizeof(nameKey), "q_m_%d", i);
        snprintf(durKey, sizeof(durKey), "q_d_%d", i);

        String name = preferences.getString(nameKey, "");
        const unsigned int duration = preferences.getUInt(durKey, 0);

        if (!name.isEmpty() && duration > 0)
        {
            m_config.queue.push_back({name, duration});
        }
    }

    preferences.end();

    if (m_onConfigChangeCallback)
    {
        m_onConfigChangeCallback(m_config);
    }
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

            int interval = server.arg("interval").toInt();

            if (interval <= 0)
            {
                interval = 30;
            }

            preferences.putUInt("interval", interval);
            preferences.putString("units", server.arg("units").c_str());

            preferences.putFloat("w_lat", server.arg("w_lat").toFloat());
            preferences.putFloat("w_lon", server.arg("w_lon").toFloat());
            preferences.putString("w_svc", server.arg("w_svc"));
            preferences.putString("w_key", server.arg("w_key"));

            // clear old saved queue
            const int oldQueueSize = preferences.getInt("q_size", 0);
            for (int i = 0; i < oldQueueSize; ++i)
            {
                preferences.remove(("q_m_" + String(i)).c_str());
                preferences.remove(("q_d_" + String(i)).c_str());
            }

            int newQueueSize = 0;
            while (true)
            {
                const String moduleArgName = "q_m_" + String(newQueueSize);
                const String durationArgName = "q_d_" + String(newQueueSize);

                if (server.hasArg(moduleArgName) && server.hasArg(durationArgName))
                {
                    const String name = server.arg(moduleArgName);
                    const unsigned int duration = server.arg(durationArgName).toInt();

                    preferences.putString(("q_m_" + String(newQueueSize)).c_str(), name);
                    preferences.putUInt(("q_d_" + String(newQueueSize)).c_str(), duration);

                    newQueueSize++;
                }
                else
                {
                    break;
                }
            }

            preferences.putInt("q_size", newQueueSize);

            preferences.end();
            loadConfiguration();

            const String response =
                "<html><head><meta name=\"darkreader-lock\"><style>body{background-color:#1e2030;color:#cad3f5;font-family:sans-serif;text-align:center;padding-top:50px;}a{color:#8aadf4;}</style></head><body><h1>Configuration Saved!</h1><p>Your settings have been updated.</p><br/><a href='/'>Go back</a></body></html>";
            server.send(200, "text/html", response);
        }
    );

    server.on(
        "/get_config", HTTP_GET, [&]()
        {
            const DeviceConfig& cfg = getConfig();
            JsonDocument doc;

            const JsonObject settings = doc["settings"].to<JsonObject>();
            settings["interval"] = cfg.interval;
            settings["units"] = cfg.units;
            settings["weather_lat"] = cfg.weather_lat;
            settings["weather_lon"] = cfg.weather_lon;
            settings["weather_service"] = cfg.weather_service;
            settings["weather_apikey"] = cfg.weather_apikey;

            const JsonArray queue = doc["queue"].to<JsonArray>();
            for (const auto& item : cfg.queue)
            {
                auto queueItem = queue.add<JsonObject>();
                queueItem["module"] = item.name;
                queueItem["duration"] = item.duration;
            }

            String json;
            serializeJson(doc, json);
            server.send(200, "application/json", json);
        }
    );
}
