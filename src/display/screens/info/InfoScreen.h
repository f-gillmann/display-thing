#pragma once

#include "display/screens/Screen.h"
#include <string>

#include "configuration/ConfigurationManager.h"

class InfoScreen final : public Screen
{
public:
    explicit InfoScreen();
    void show(DisplayThing& displayThing) override;
    void setConfig(const DeviceConfig& deviceConfig) override;

private:
    DeviceConfig m_config;
};
