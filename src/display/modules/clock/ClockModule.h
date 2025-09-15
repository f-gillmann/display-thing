#pragma once

#include "display/modules/Module.h"
#include "display/time/TimeManager.h"

class ClockModule final : public Module
{
public:
    explicit ClockModule(TimeManager& timeManager);

    void setConfig(const DeviceConfig& deviceConfig) override;
    void update() override;
    void show(DisplayThing& displayThing) override;

private:
    TimeManager& m_timeManager;
    DeviceConfig m_config;

    tm m_timeInfo{};
    char m_timeString[9]{};
    char m_dateString[32]{};
    bool m_hasTime = false;
};
