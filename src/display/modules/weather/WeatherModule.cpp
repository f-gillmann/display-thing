//
// Created by Florian on 08/08/2025.
//

#include "WeatherModule.h"

#include "util.hpp"
#include "display/services/wheater/OpenMeteoService.h"
#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSansBold12pt7b.h"

std::unique_ptr<WeatherService> createWeatherService(const std::string& serviceName)
{
    if (serviceName == "openmeteo")
    {
        return make_unique<OpenMeteoService>();
    }

    return nullptr;
}

WeatherModule::WeatherModule() = default;

void WeatherModule::setConfig(const DeviceConfig& deviceConfig)
{
    m_config = deviceConfig;
    m_weatherService = createWeatherService(m_config.weather_service.c_str());

    if (m_config.weather_lat != 0.0f && m_config.weather_lon != 0.0f)
    {
        m_isConfigured = true;
    }
    else
    {
        m_isConfigured = false;
    }
}

void WeatherModule::update()
{
    if (!m_isConfigured || !m_weatherService) return;

    Serial.println("WeatherModule: Updating data...");
    m_weatherData = m_weatherService->fetchWeatherData(
        m_config.weather_lat,
        m_config.weather_lon,
        m_config.weather_apikey.c_str(),
        m_config.units.c_str()
    );
}

void WeatherModule::show(DisplayThing& displayThing) {
    auto& display = displayThing.getDisplay();
    display.setFullWindow();
    display.firstPage();

    do {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);

        if (m_isConfigured) {
            if (m_weatherData.success) {
                display.setFont(&FreeSansBold12pt7b);
                display.setCursor(10, 30);
                display.print("Current Weather");

                display.setFont(&FreeSans9pt7b);
                display.setCursor(10, 60);
                display.print("Temperature: ");
                display.print(m_weatherData.temperature, 1);
                display.print(m_config.units == "metric" ? " C" : " F");

                display.setCursor(10, 80);
                display.print("Condition Code: ");
                display.print(m_weatherData.weather_code);
            } else {
                display.setFont(&FreeSansBold12pt7b);
                display.setCursor(10, 30);
                display.print("Weather Data Error");

                display.setFont(&FreeSans9pt7b);
                display.setCursor(10, 60);
                display.print("Could not fetch weather data.");
            }
        } else {
            display.setFont(&FreeSansBold12pt7b);
            display.setCursor(10, 30);
            display.print("Weather Module Not Configured");

            display.setFont(&FreeSans9pt7b);
            display.setCursor(10, 60);
            display.print("Please set your latitude and longitude");
            display.setCursor(10, 80);
            display.print("in the web configuration portal.");
        }
    } while (display.nextPage());
}
