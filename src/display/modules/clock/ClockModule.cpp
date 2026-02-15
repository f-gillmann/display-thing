#include "ClockModule.h"
#include "fonts/fonts.h"
#include "../ModuleHelpers.h"

ClockModule::ClockModule(TimeManager& timeManager)
    : m_timeManager(timeManager)
{
}

void ClockModule::setConfig(const DeviceConfig& deviceConfig)
{
    m_config = deviceConfig;
}

void ClockModule::update()
{
    const NTPClient& ntpClient = m_timeManager.getTimeClient();

    // check if the unix epoch time is recent, indicating a successful sync (probably).
    // (1757800000 = Sat Sep 13 2025 21:46:40 GMT+0000).
    if (ntpClient.getEpochTime() > 1757800000L)
    {
        if (!m_hasTime)
            m_hasTime = true;

        const auto raw_time = static_cast<time_t>(ntpClient.getEpochTime());
        m_timeInfo = *gmtime(&raw_time);

        // Format time string based on config settings
        const char* time_fmt;
        if (m_config.modules.clock.show_seconds)
        {
            time_fmt = m_config.clock_format == "12" ? "%I:%M:%S" : "%H:%M:%S";
        }
        else
        {
            time_fmt = m_config.clock_format == "12" ? "%I:%M" : "%H:%M";
        }
        strftime(m_timeString, sizeof(m_timeString), time_fmt, &m_timeInfo);
        strftime(m_dateString, sizeof(m_dateString), "%A, %B %d", &m_timeInfo);
    }
    else
    {
        m_hasTime = false;
        m_timeString[0] = '\0'; // clear strings if we don't have a valid time
    }
}

bool ClockModule::needsFrequentUpdates() const
{
    return m_config.modules.clock.show_seconds;
}

unsigned long ClockModule::getUpdateInterval() const
{
    return m_config.modules.clock.show_seconds ? 1000 : 60000;
}

void ClockModule::drawContent(DisplayThing& displayThing, const bool usePartialUpdate)
{
    auto& display = displayThing.getDisplay();

    if (usePartialUpdate)
    {
        display.setPartialWindow(0, 0, display.width(), display.height());
    }
    else
    {
        display.setFullWindow();
    }

    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);

        if (m_hasTime)
        {
            // Determine fonts and layout based on size setting
            // Using monospace fonts to prevent digit-width changes during updates
            const GFXfont* timeFont;
            const GFXfont* dateFont;
            const GFXfont* tzFont;
            int lineSpacing;

            if (m_config.modules.clock.size == "xl")
            {
                timeFont = &FreeMonoBold48pt7b;
                dateFont = &FreeMonoBold12pt7b;
                tzFont = &FreeMonoBold9pt7b;
                lineSpacing = 60;
            }
            else if (m_config.modules.clock.size == "large")
            {
                timeFont = &FreeMonoBold36pt7b;
                dateFont = &FreeMonoBold12pt7b;
                tzFont = &FreeMonoBold9pt7b;
                lineSpacing = 50;
            }
            else if (m_config.modules.clock.size == "medium")
            {
                timeFont = &FreeMonoBold24pt7b;
                dateFont = &FreeMonoBold12pt7b;
                tzFont = &FreeMonoBold9pt7b;
                lineSpacing = 40;
            }
            else
            {
                // small
                timeFont = &FreeMonoBold18pt7b;
                dateFont = &FreeMonoBold12pt7b;
                tzFont = &FreeMonoBold9pt7b;
                lineSpacing = 35;
            }

            int blockHeight = lineSpacing;
            if (m_config.modules.clock.show_date) blockHeight += 25;
            if (m_config.modules.clock.show_timezone) blockHeight += 25;

            // prevent horizontal jitter
            char max_time_string[32];
            if (m_config.clock_format == "12")
            {
                if (m_config.modules.clock.show_seconds)
                    strcpy(max_time_string, "12:59:59 PM");
                else
                    strcpy(max_time_string, "12:59 PM");
            }
            else
            {
                if (m_config.modules.clock.show_seconds)
                    strcpy(max_time_string, "23:59:59");
                else
                    strcpy(max_time_string, "23:59");
            }

            // calculate max time text height for vertical centering
            int16_t tx1, ty1;
            uint16_t tw, th;
            display.setFont(timeFont);
            display.getTextBounds(max_time_string, 0, 0, &tx1, &ty1, &tw, &th);
            const int blockStartY = (display.height() - blockHeight) / 2 + th / 2;
            int currentY = blockStartY;

            // draw time
            char time_with_ampm[32];
            if (m_config.clock_format == "12")
            {
                char am_pm[3];
                strftime(am_pm, sizeof(am_pm), "%p", &m_timeInfo);
                snprintf(time_with_ampm, sizeof(time_with_ampm), "%s %s", m_timeString, am_pm);
                // Pad to max length
                int pad = strlen(max_time_string) - strlen(time_with_ampm);
                char padded_time[32];
                snprintf(padded_time, sizeof(padded_time), "%s%*s", time_with_ampm, pad, "");
                drawCenteredString(displayThing, padded_time, timeFont, 0, currentY, true, false);
            }
            else
            {
                int pad = strlen(max_time_string) - strlen(m_timeString);
                char padded_time[32];
                snprintf(padded_time, sizeof(padded_time), "%s%*s", m_timeString, pad, "");
                drawCenteredString(displayThing, padded_time, timeFont, 0, currentY, true, false);
            }
            currentY += lineSpacing;

            // draw date if enabled
            if (m_config.modules.clock.show_date)
            {
                drawCenteredString(displayThing, m_dateString, dateFont, 0, currentY, true, false);
                currentY += 25;
            }

            // draw timezone if enabled
            if (m_config.modules.clock.show_timezone)
            {
                char tz_str[12];
                const long offset_sec = m_config.time_offset;
                const int hours = offset_sec / 3600;
                const int minutes = offset_sec % 3600 / 60;
                snprintf(tz_str, sizeof(tz_str), "UTC%+03d:%02d", hours, abs(minutes));
                drawCenteredString(displayThing, tz_str, tzFont, 0, currentY, true, false);
            }
        }
        else
        {
            // vertically center waiting message
            const int y = (display.height() - 24) / 2;
            drawCenteredString(displayThing, "Waiting for time sync...", &FreeMonoBold12pt7b, 0, y, true, false);
        }
    }
    while (display.nextPage());
}
