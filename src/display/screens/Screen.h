#pragma once

#include "DisplayThing.h"
#include "display/configuration/ConfigurationManager.h"

class Screen
{
public:
    virtual ~Screen() = default;
    virtual void show(DisplayThing& displayThing) = 0;
    virtual void setConfig(const DeviceConfig& deviceConfig) = 0;
    virtual void update() {}
};
