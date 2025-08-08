//
// Created by Florian on 08/08/2025.
//

#include "WeatherModule.h"

#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSansBold12pt7b.h"

WeatherModule::WeatherModule() = default;

void WeatherModule::setConfig(const DeviceConfig& deviceConfig)
{
    m_config = deviceConfig;

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
    if (!m_isConfigured) return;
}

void WeatherModule::show(DisplayThing& displayThing)
{
    auto& display = displayThing.getDisplay();
    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);

        if (m_isConfigured)
        {
            display.setFont(&FreeSansBold12pt7b);
            display.setCursor(10, 30);
            display.print("Weather Module");
        }
        else
        {
            display.setFont(&FreeSansBold12pt7b);
            display.setCursor(10, 30);
            display.print("Weather Module Not Configured");

            display.setFont(&FreeSans9pt7b);
            display.setCursor(10, 60);
            display.print("Please set your latitude and longitude");
            display.setCursor(10, 80);
            display.print("in the web configuration portal.");
        }
    }
    while (display.nextPage());
}
