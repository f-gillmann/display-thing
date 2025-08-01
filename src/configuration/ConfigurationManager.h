#pragma once

#include <functional>
#include "DisplayThing.h"

struct DeviceConfig
{
    uint32_t interval;
    std::string units;
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
