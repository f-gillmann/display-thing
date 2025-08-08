#pragma once
#include "display/modules/Module.h"

class WeatherModule final : public Module {
public:
    explicit WeatherModule();
    void show(DisplayThing& displayThing) override;
    void setConfig(const DeviceConfig& deviceConfig) override;
    void update() override;

private:
    DeviceConfig m_config;
    bool m_isConfigured = false;
};
