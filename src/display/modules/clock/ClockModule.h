#pragma once

#include "display/modules/Module.h"
#include "ezTime.h"

class ClockModule final : public Module {
public:
    explicit ClockModule();
    void show(DisplayThing& displayThing) override;
    void setConfig(const DeviceConfig& deviceConfig) override;
    void update() override;

private:
    DeviceConfig m_config;
    bool m_isConfigured = false;
    tm m_timeInfo;

    // buffers to hold the formatted time and date strings
    char m_timeString[9]{};    // e.g., "23:01" or "11:01"
    char m_dateString[30]{};   // e.g., "Friday, August 08"
};
