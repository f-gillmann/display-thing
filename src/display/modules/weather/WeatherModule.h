#pragma once

#include "display/modules/Module.h"
#include "display/services/wheater/WheaterService.h"

class WeatherModule final : public Module
{
public:
    explicit WeatherModule();
    void setConfig(const DeviceConfig& deviceConfig) override;
    void update() override;

protected:
    void drawContent(DisplayThing& displayThing, bool usePartialUpdate) override;

private:
    DeviceConfig m_config;
    bool m_isConfigured = false;
    std::unique_ptr<WeatherService> m_weatherService;
    WeatherData m_weatherData;
};
