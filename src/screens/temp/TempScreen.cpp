#include "TempScreen.h"

#include <WiFi.h>
#include "../../globals.h"
#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSansBold12pt7b.h"

void show_temp_screen()
{
    const String ip_address = WiFi.localIP().toString();
    const unsigned long uptime_ms = millis();

    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(10, 30);
        display.print("Temporary Screen");
        display.setFont(&FreeSans9pt7b);
        display.setCursor(10, 80);
        display.print("IP Address: ");
        display.print(ip_address);
        display.setCursor(10, 110);
        display.print("Uptime: ");
        display.print(uptime_ms / 60000);
        display.print(" minutes");

    } while (display.nextPage());

    Serial.println("Temporary screen updated.");
}
