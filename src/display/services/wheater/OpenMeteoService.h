#pragma once
#include "WheaterService.h"

class OpenMeteoService final : public WeatherService
{
public:
    WeatherData fetchWeatherData(float lat, float lon, const std::string& apiKey, const std::string& units) override;
};
