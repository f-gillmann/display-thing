#pragma once

#include <functional>
#include "DisplayThing.h"

struct ModuleQueueItem
{
    String name;
    unsigned int duration;
};

struct DeviceConfig
{
    uint32_t interval;
    String units;

    std::vector<ModuleQueueItem> queue;

    float weather_lat;
    float weather_lon;
    String weather_service;
    String weather_apikey;
};


class ConfigurationManager
{
public:
    using ConfigChangeCallback = std::function<void(DeviceConfig&)>;

    explicit ConfigurationManager(DisplayThing& displayThing);
    auto registerHandlers() -> void;
    const DeviceConfig& getConfig() const;

    void onConfigChanged(const ConfigChangeCallback& callback);

private:
    void loadConfiguration();

    DisplayThing& displayThing;
    DeviceConfig m_config;

    ConfigChangeCallback m_onConfigChangeCallback = nullptr;
};
