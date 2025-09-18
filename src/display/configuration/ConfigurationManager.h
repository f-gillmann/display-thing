#pragma once

#include <functional>
#include <vector>
#include <WString.h>

class DisplayThing;

struct QueueItem
{
    String name;
    unsigned int duration;
};

struct DeviceConfig
{
    String units;
    String clock_format;
    int time_offset;
    String timezone;
    float weather_lat;
    float weather_lon;
    String weather_service;
    String weather_apikey;
    std::vector<QueueItem> queue;
};

class ConfigurationManager
{
public:
    explicit ConfigurationManager(DisplayThing& displayThing);
    using ConfigChangeCallback = std::function<void(const DeviceConfig&)>;
    void onConfigChanged(const ConfigChangeCallback& callback);
    const DeviceConfig& getConfig() const;
    void registerHandlers();
    void loadConfiguration();
    void logConfiguration() const;

private:
    DisplayThing& displayThing;
    DeviceConfig m_config;
    std::vector<ConfigChangeCallback> m_callbacks;
};
