#include "OpenMeteoService.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "Logger.hpp"

WeatherData OpenMeteoService::fetchWeatherData(
    const float lat, const float lon, const std::string& apiKey, const std::string& units
)
{
    WeatherData data;
    HTTPClient http;

    String url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(lat, 2) +
        "&longitude=" + String(lon, 2) +
        "&current=temperature_2m,weather_code";

    if (units == "metric")
    {
        url += "&temperature_unit=celsius";
    }
    else
    {
        url += "&temperature_unit=fahrenheit";
    }

    LOG_INFO("Fetching weather info...");

    http.begin(url.c_str());
    const int httpCode = http.GET();

    if (httpCode > 0)
    {
        String response = http.getString();

        JsonDocument jsonDoc;
        const DeserializationError error = deserializeJson(jsonDoc, response);

        if (!error)
        {
            data.temperature = jsonDoc["current"]["temperature_2m"].as<float>();
            data.weather_code = jsonDoc["current"]["weather_code"].as<int>();
            data.success = true;
        }
        else
        {
            LOG_ERROR("Error parsing JSON");
        }
    }
    else
    {
        LOG_ERROR("HTTP GET failed, error: %s", HTTPClient::errorToString(httpCode).c_str());
    }

    http.end();
    return data;
}
