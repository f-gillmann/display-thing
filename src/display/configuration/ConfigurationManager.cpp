#include "ConfigurationManager.h"

#include <Preferences.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "config.h"
#include "Logger.hpp"
#include "display/DisplayManager.h"
#include "display/web/configuration_portal.html.h"

ConfigurationManager::ConfigurationManager(DisplayThing& displayThing) : displayThing(displayThing)
{
    loadConfiguration();
}

void ConfigurationManager::onConfigChanged(const ConfigChangeCallback& callback)
{
    m_callbacks.push_back(callback);
}

const DeviceConfig& ConfigurationManager::getConfig() const
{
    return m_config;
}

void ConfigurationManager::loadConfiguration()
{
    LOG_INFO("Loading configuration...");

    // keys can't be too long since there is a length limit.
    auto& preferences = displayThing.getPreferences();
    preferences.begin(PREFERENCES_DEVICE_CONFIG);

    // load general settings
    m_config.units = preferences.getString("un", "metric").c_str(); // units
    m_config.clock_format = preferences.getString("cl_fo", "24"); // clock_format
    m_config.time_offset = preferences.getInt("toff", 0); // time_offset
    m_config.timezone = preferences.getString("tz", "Europe/Berlin"); // timezone

    // load weather settings
    m_config.weather_lat = preferences.getFloat("w_lat", 51.16f); // weather_lat
    m_config.weather_lon = preferences.getFloat("w_lon", 10.45f); // weather_lon
    m_config.weather_service = preferences.getString("w_svc", "openmeteo"); // weather_service
    m_config.weather_apikey = preferences.getString("w_key", ""); // weather_apikey

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

    if (!m_callbacks.empty())
    {
        for (auto& cb : m_callbacks)
        {
            cb(m_config);
        }
    }
}

void ConfigurationManager::logConfiguration() const
{
    Serial.println(F("--- Current Configuration ---"));

    Serial.println(F("[General]"));
    Serial.printf("  Units: %s\n", m_config.units.c_str());
    Serial.printf("  Clock Format: %s\n", m_config.clock_format.c_str());
    Serial.printf("  Time Offset: %i\n", m_config.time_offset);
    Serial.printf("  Timezone: %s\n", m_config.timezone.c_str());

    Serial.println(F("\n[Weather]"));
    Serial.printf("  Latitude: %f\n", m_config.weather_lat);
    Serial.printf("  Longitude: %f\n", m_config.weather_lon);
    Serial.printf("  Service: %s\n", m_config.weather_service.c_str());
    Serial.printf("  API Key: %s\n", m_config.weather_apikey.isEmpty() ? "Not Set" : "Set");

    Serial.println(F("\n[Queue Items]"));

    if (m_config.queue.empty())
    {
        Serial.println("  Queue is empty.");
    }
    else
    {
        int itemIndex = 0;
        for (const auto& item : m_config.queue)
        {
            Serial.printf(
                "  [%d] Name: %s, Duration: %u\n",
                itemIndex++,
                item.name.c_str(),
                item.duration
            );
        }
    }

    Serial.println(F("-----------------------------"));
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
            if (!preferences.begin(PREFERENCES_DEVICE_CONFIG))
            {
                server.send(500, "application/json", R"({"error":"Could not open storage"})");
                return;
            }

            /*
            if (!server.hasArg("timezone") || server.arg("timezone").isEmpty()) {
                preferences.end();
                server.send(400, "application/json", R"({"error":"Missing timezone"})");
                return;
            }
            */

            int time_offset = server.arg("time_offset").toInt();

            if (!time_offset)
            {
                time_offset = 0;
            }

            preferences.putString("un", server.arg("units").c_str()); // units
            preferences.putString("cl_fo", server.arg("clock_format").c_str()); // clock_format
            preferences.putInt("toff", time_offset); // time_offset
            preferences.putString("tz", server.arg("timezone").c_str());

            preferences.putFloat("w_lat", server.arg("weather_lat").toFloat()); // weather_lat
            preferences.putFloat("w_lon", server.arg("weather_lon").toFloat()); // weather_lon
            preferences.putString("w_svc", server.arg("weather_service")); // weather_service
            preferences.putString("w_key", server.arg("weather_apikey")); // weather_apikey

            // clear old saved queue
            const int oldQueueSize = preferences.getInt("q_size", 0);
            for (int i = 0; i < oldQueueSize; ++i)
            {
                preferences.remove(("q_m_" + String(i)).c_str()); // queue_module_index
                preferences.remove(("q_d_" + String(i)).c_str()); // queue_duration_index
            }

            int newQueueSize = 0;
            while (true)
            {
                const String moduleArgName = "q_m_" + String(newQueueSize); // queue_module_index
                const String durationArgName = "q_d_" + String(newQueueSize); // queue_duration_index

                if (server.hasArg(moduleArgName) && server.hasArg(durationArgName))
                {
                    const String name = server.arg(moduleArgName);
                    const unsigned int duration = server.arg(durationArgName).toInt();

                    preferences.putString(("q_m_" + String(newQueueSize)).c_str(), name); // queue_module_index
                    preferences.putUInt(("q_d_" + String(newQueueSize)).c_str(), duration); // queue_duration_index

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

            server.send(200, "application/json", R"({"status":"success"})");
        }
    );

    server.on(
        "/get_config", HTTP_GET, [&]()
        {
            const DeviceConfig& cfg = getConfig();
            JsonDocument doc;

            const JsonObject settings = doc["settings"].to<JsonObject>();
            settings["units"] = cfg.units;
            settings["clock_format"] = cfg.clock_format;
            settings["time_offset"] = cfg.time_offset;
            settings["timezone"] = cfg.timezone;
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

    server.on(
        "/favicon.ico", [&]()
        {
            server.send(204);
        }
    );

    server.onNotFound(
        [&]()
        {
            server.send(404, "text/plain", "Not found");
        }
    );
}
