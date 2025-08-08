#include "WelcomeScreen.h"

#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

WelcomeScreen::WelcomeScreen() = default;

void WelcomeScreen::setConfig(const DeviceConfig& deviceConfig)
{
}

void WelcomeScreen::show(DisplayThing& displayThing)
{
    auto& display = displayThing.getDisplay();
    display.setFullWindow();
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);

        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(10, 30);
        display.print("DisplayThing Ready!");

        display.setFont(&FreeSans9pt7b);
        display.setCursor(10, 60);
        display.print("The module queue is empty.");
        display.setCursor(10, 80);
        display.print("Please add modules in the web portal to get started.");

        display.setCursor(10, 120);
        display.print("Visit its IP address to access the configuration panel:");
        display.setCursor(10, 140);
        display.setFont(&FreeSansBold9pt7b);
        display.print(WiFi.localIP().toString());
    }
    while (display.nextPage());
}
