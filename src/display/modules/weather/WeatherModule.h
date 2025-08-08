#pragma once
#include "display/modules/Module.h"
#include "display/services/wheater/WheaterService.h"

class WeatherModule final : public Module {
public:
    explicit WeatherModule();
    void show(DisplayThing& displayThing) override;
    void setConfig(const DeviceConfig& deviceConfig) override;
    void update() override;

private:
    DeviceConfig m_config;
    bool m_isConfigured = false;
    std::unique_ptr<WeatherService> m_weatherService;
    WeatherData m_weatherData;
};
