#pragma once

#include "DisplayThing.h"

struct DeviceConfig
{
    uint32_t interval;
    std::string units;
    std::string apiKey;
};

class ConfigurationManager
{
public:
    explicit ConfigurationManager(DisplayThing& displayThing);
    auto registerHandlers() -> void;
    const DeviceConfig& getConfig() const;

private:
    void loadConfiguration();

    DisplayThing& displayThing;
    DeviceConfig m_config;
};
