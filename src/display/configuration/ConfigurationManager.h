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

struct ClockModuleConfig
{
    String size;
    bool show_seconds{};
    bool show_date{};
    bool show_timezone{};
};

struct WeatherModuleConfig
{
    float lat{};
    float lon{};
    String service;
    String apikey;
};

struct ModulesConfig
{
    ClockModuleConfig clock;
    WeatherModuleConfig weather;
};

struct DeviceConfig
{
    String units;
    String clock_format;
    int time_offset{};
    String timezone;
    unsigned int full_refresh_interval{}; // Minutes between full refreshes (0 = disabled)
    ModulesConfig modules;
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
