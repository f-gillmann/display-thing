#include "InfoScreen.h"

#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

#include "display/DisplayManager.h"

InfoScreen::InfoScreen() = default;

void InfoScreen::show(DisplayThing& displayThing)
{
    auto& display = displayThing.getDisplay();

    display.setFullWindow();
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);

        // title
        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(10, 20);
        display.print("Current Configuration");

        constexpr int label_x = 10;
        constexpr int value_x = 150;
        constexpr int line_height = 25;
        int current_y = 50;

        // update interval
        display.setFont(&FreeSans9pt7b);
        display.setCursor(label_x, static_cast<int16_t>(current_y));
        display.print("Update Interval:");

        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(value_x, static_cast<int16_t>(current_y));
        display.print(String(m_config.interval / 1000) + " seconds");
        current_y += line_height;

        // units
        display.setFont(&FreeSans9pt7b);
        display.setCursor(label_x, static_cast<int16_t>(current_y));
        display.print("Units:");

        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(value_x, static_cast<int16_t>(current_y));
        display.print(m_config.units.c_str());
        current_y += line_height;

        // weather settings
        current_y += 10;
        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(label_x, static_cast<int16_t>(current_y));
        display.print("Weather");
        current_y += line_height;

        // latitude
        display.setFont(&FreeSans9pt7b);
        display.setCursor(label_x, static_cast<int16_t>(current_y));
        display.print("Latitude:");
        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(value_x, static_cast<int16_t>(current_y));
        display.print(String(m_config.weather_lat, 4));
        current_y += line_height;

        // longitude
        display.setFont(&FreeSans9pt7b);
        display.setCursor(label_x, static_cast<int16_t>(current_y));
        display.print("Longitude:");
        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(value_x, static_cast<int16_t>(current_y));
        display.print(String(m_config.weather_lon, 4));
        current_y += line_height;

        // service
        display.setFont(&FreeSans9pt7b);
        display.setCursor(label_x, static_cast<int16_t>(current_y));
        display.print("Service:");
        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(value_x, static_cast<int16_t>(current_y));
        display.print(m_config.weather_service.c_str());
        current_y += line_height;

        // api key
        display.setFont(&FreeSans9pt7b);
        display.setCursor(label_x, static_cast<int16_t>(current_y));
        display.print("API Key:");
        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(value_x, static_cast<int16_t>(current_y));
        display.print(m_config.weather_apikey.isEmpty() ? "Not Set" : "Set");
        current_y += line_height;


        // display queue
        current_y += 10;
        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(label_x, static_cast<int16_t>(current_y));
        display.print("Display Queue");
        current_y += line_height;

        if (m_config.queue.empty())
        {
            display.setFont(&FreeSans9pt7b);
            display.setCursor(label_x, static_cast<int16_t>(current_y));
            display.print("Queue is empty.");
        }
        else
        {
            for (const auto& item : m_config.queue)
            {
                display.setFont(&FreeSans9pt7b);
                display.setCursor(label_x, static_cast<int16_t>(current_y));
                display.print(item.name.c_str());

                display.setFont(&FreeSansBold9pt7b);
                display.setCursor(value_x, static_cast<int16_t>(current_y));
                display.print(String(item.duration) + "s");
                current_y += line_height;
            }
        }
    }
    while (display.nextPage());
}

void InfoScreen::setConfig(const DeviceConfig& deviceConfig)
{
    m_config = deviceConfig;
}
