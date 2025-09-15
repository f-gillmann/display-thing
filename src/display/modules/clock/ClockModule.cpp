#include "ClockModule.h"
#include <string>
#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSansBold12pt7b.h"
#include "Fonts/FreeSansBold24pt7b.h"

void drawCenteredString(DisplayThing& displayThing, const char* text, const GFXfont* font, int y);

ClockModule::ClockModule(TimeManager& timeManager)
    : m_timeManager(timeManager), m_config()
{
}

void ClockModule::setConfig(const DeviceConfig& deviceConfig)
{
    m_config = deviceConfig;
}

void ClockModule::update()
{
    const NTPClient& ntpClient = m_timeManager.getTimeClient();

    // check if the unix epoch time is recent, indicating a successful sync.
    // (1757800000 = Sat Sep 13 2025 21:46:40 GMT+0000).
    if (ntpClient.getEpochTime() > 1757800000L)
    {
        if (!m_hasTime)
            m_hasTime = true;

        const auto raw_time = static_cast<time_t>(ntpClient.getEpochTime());
        m_timeInfo = *gmtime(&raw_time);

        const char* time_fmt = (m_config.clock_format == "12") ? "%I:%M" : "%H:%M";
        strftime(m_timeString, sizeof(m_timeString), time_fmt, &m_timeInfo);
        strftime(m_dateString, sizeof(m_dateString), "%A, %B %d", &m_timeInfo);
    }
    else
    {
        m_hasTime = false;
        m_timeString[0] = '\0'; // clear strings if we don't have a valid time
    }
}

void ClockModule::show(DisplayThing& displayThing)
{
    auto& display = displayThing.getDisplay();
    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);

        if (m_hasTime)
        {
            constexpr int time_y = 70;
            constexpr int date_y = 110;
            constexpr int timezone_y = 135;

            if (m_config.clock_format == "12")
            {
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
                display.setCursor(static_cast<int16_t>(start_x), time_y);
                display.print(m_timeString);

                display.setFont(&FreeSansBold12pt7b);
                display.setCursor(static_cast<int16_t>(start_x + w_time + padding), time_y);
                display.print(am_pm);
            }
            else
            {
                drawCenteredString(displayThing, m_timeString, &FreeSansBold24pt7b, time_y);
            }

            drawCenteredString(displayThing, m_dateString, &FreeSansBold12pt7b, date_y);

            char tz_str[12];
            const long offset_sec = m_config.time_offset;
            const int hours = offset_sec / 3600;
            const int minutes = (offset_sec % 3600) / 60;

            snprintf(tz_str, sizeof(tz_str), "UTC%+03d:%02d", hours, abs(minutes));
            drawCenteredString(displayThing, tz_str, &FreeSans9pt7b, timezone_y);
        }
        else
        {
            drawCenteredString(displayThing, "Waiting for time sync...", &FreeSansBold12pt7b, 80);
        }
    }
    while (display.nextPage());
}

void drawCenteredString(DisplayThing& displayThing, const char* text, const GFXfont* font, const int y)
{
    auto& display = displayThing.getDisplay();
    int16_t x1, y1;
    uint16_t w, h;

    display.setFont(font);
    display.getTextBounds(text, 0, static_cast<int16_t>(y), &x1, &y1, &w, &h);

    const int x = (display.width() - w) / 2;
    display.setCursor(static_cast<int16_t>(x), static_cast<int16_t>(y));
    display.print(text);
}
