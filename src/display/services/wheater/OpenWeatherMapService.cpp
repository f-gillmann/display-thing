#include "OpenWeatherMapService.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

WeatherData OpenWeatherMapService::fetchWeatherData(
    const float lat, const float lon, const std::string& apiKey, const std::string& units
)
{
    WeatherData data;
    HTTPClient http;

    if (apiKey.empty())
    {
        Serial.println("OpenWeatherMap Error: API Key is not set.");
        data.success = false;
        return data;
    }

    const String url = "https://api.openweathermap.org/data/2.5/weather?lat=" + String(lat, 2) +
        "&lon=" + String(lon, 2) +
        "&appid=" + String(apiKey.c_str()) +
        "&units=" + String(units.c_str());

    Serial.println("Fetching weather from: " + url);

    http.begin(url);
    const int httpCode = http.GET();

    if (httpCode > 0)
    {
        {
            String response = http.getString();

            JsonDocument jsonDoc;
            const DeserializationError error = deserializeJson(jsonDoc, response);

            if (error)
            {
                data.temperature = jsonDoc["main"]["temp"];
                data.weather_code = jsonDoc["weather"][0]["id"];
                data.success = true;
            }
            else
            {
                Serial.print("Error parsing JSON: ");
                Serial.println(error.c_str());
            }
        }
    }
    else
    {
        Serial.printf("HTTP GET failed, error: %s\n", HTTPClient::errorToString(httpCode).c_str());
    }

    http.end();
    return data;
}
