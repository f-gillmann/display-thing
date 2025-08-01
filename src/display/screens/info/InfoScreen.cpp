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
        int current_y = 50;

        // update interval
        display.setFont(&FreeSans9pt7b);
        display.setCursor(label_x, static_cast<int16_t>(current_y));
        display.print("Update Interval:");

        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(value_x, static_cast<int16_t>(current_y));
        display.print(String(m_config.interval / 1000) + " seconds");
        current_y += 25;

        // units
        display.setFont(&FreeSans9pt7b);
        display.setCursor(label_x, static_cast<int16_t>(current_y));
        display.print("Units:");

        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(value_x, static_cast<int16_t>(current_y));
        display.print(m_config.units.c_str());
    }
    while (display.nextPage());
}

void InfoScreen::setConfig(const DeviceConfig& deviceConfig)
{
    m_config = deviceConfig;
}
