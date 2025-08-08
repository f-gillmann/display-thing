#include "ClockModule.h"
#include <time.h>
#include <string>
#include <algorithm>
#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSansBold12pt7b.h"
#include "Fonts/FreeSansBold24pt7b.h"

void drawCenteredString(DisplayThing& displayThing, const char* text, const GFXfont* font, int y);

ClockModule::ClockModule() = default;

void ClockModule::setConfig(const DeviceConfig& deviceConfig)
{
    m_config = deviceConfig;

    if (!m_config.timezone.isEmpty())
    {
        configTzTime(m_config.timezone.c_str(), "pool.ntp.org", "time.nist.gov");
        m_isConfigured = true;
        Serial.printf("ClockModule: Timezone configured to '%s'\n", m_config.timezone.c_str());
    }
    else
    {
        m_isConfigured = false;
        Serial.println("ClockModule: Not configured, timezone is missing.");
    }
}

void ClockModule::update()
{
    if (!m_isConfigured) return;

    if (getLocalTime(&m_timeInfo, 1000))
    {
        const char* time_fmt = (m_config.clock_format == "12") ? "%I:%M" : "%H:%M";
        strftime(m_timeString, sizeof(m_timeString), time_fmt, &m_timeInfo);

        strftime(m_dateString, sizeof(m_dateString), "%A, %B %d", &m_timeInfo);
    }
    else
    {
        Serial.println("ClockModule: Failed to obtain time.");
        m_timeString[0] = '\0';
        m_dateString[0] = '\0';
    }
}

void ClockModule::show(DisplayThing& displayThing) {
    auto& display = displayThing.getDisplay();
    display.setFullWindow();
    display.firstPage();

    do {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);

        if (m_isConfigured) {
            if (m_timeString[0] != '\0') {
                constexpr int time_y = 70;
                constexpr int date_y = 110;
                constexpr int timezone_y = 135;

                if (m_config.clock_format == "12") {
                    char am_pm[3];
                    strftime(am_pm, sizeof(am_pm), "%p", &m_timeInfo);

                    int16_t x1, y1;
                    uint16_t w_time, h_time, w_ampm, h_ampm;

                    display.setFont(&FreeSansBold24pt7b);
                    display.getTextBounds(m_timeString, 0, 0, &x1, &y1, &w_time, &h_time);
                    display.setFont(&FreeSansBold12pt7b);
                    display.getTextBounds(am_pm, 0, 0, &x1, &y1, &w_ampm, &h_ampm);

                    constexpr int padding = 5;
                    const int total_width = w_time + padding + w_ampm;
                    const int start_x = (display.width() - total_width) / 2;

                    display.setFont(&FreeSansBold24pt7b);
                    display.setCursor(start_x, time_y);
                    display.print(m_timeString);

                    display.setFont(&FreeSansBold12pt7b);
                    display.setCursor(start_x + w_time + padding, time_y);
                    display.print(am_pm);

                } else {
                    drawCenteredString(displayThing, m_timeString, &FreeSansBold24pt7b, time_y);
                }

                drawCenteredString(displayThing, m_dateString, &FreeSansBold12pt7b, date_y);

                std::string tz_str = m_config.timezone.c_str();
                std::replace(tz_str.begin(), tz_str.end(), '_', ' ');
                drawCenteredString(displayThing, tz_str.c_str(), &FreeSans9pt7b, timezone_y);

            } else {
                drawCenteredString(displayThing, "Waiting for time sync...", &FreeSansBold12pt7b, 80);
            }
        } else {
            drawCenteredString(displayThing, "Clock Not Configured", &FreeSansBold12pt7b, 60);
            drawCenteredString(displayThing, "Please set your timezone", &FreeSans9pt7b, 90);
            drawCenteredString(displayThing, "in the web portal.", &FreeSans9pt7b, 110);
        }
    } while (display.nextPage());
}

void drawCenteredString(DisplayThing& displayThing, const char* text, const GFXfont* font, const int y) {
    auto& display = displayThing.getDisplay();

    int16_t x1, y1;
    uint16_t w, h;
    display.setFont(font);
    display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
    const int x = (display.width() - w) / 2;
    display.setCursor(x, y);
    display.print(text);
}