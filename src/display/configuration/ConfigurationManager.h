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
    unsigned int interval;
    String units;
    String clock_format;
    unsigned int time_offset;
    float weather_lat;
    float weather_lon;
    String weather_service;
    String weather_apikey;
    std::vector<QueueItem> queue;
};

using ConfigChangeCallback = std::function<void(const DeviceConfig&)>;

class ConfigurationManager
{
public:
    explicit ConfigurationManager(DisplayThing& displayThing);
    void onConfigChanged(const ConfigChangeCallback& callback);
    const DeviceConfig& getConfig() const;
    void registerHandlers();
    void loadConfiguration();
    void logConfiguration() const;

private:
    DisplayThing& displayThing;
    DeviceConfig m_config;
    ConfigChangeCallback m_onConfigChangeCallback;
};
