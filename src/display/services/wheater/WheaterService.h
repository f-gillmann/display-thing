#pragma once

#include "WeatherData.h"
#include <string>

class WeatherService {
public:
    virtual ~WeatherService() = default;
    virtual WeatherData fetchWeatherData(float lat, float lon, const std::string& apiKey, const std::string& units) = 0;
};
