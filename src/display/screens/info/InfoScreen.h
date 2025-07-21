#pragma once

#include "display/screens/Screen.h"
#include <string>

#include "configuration/ConfigurationManager.h"

class InfoScreen final : public Screen
{
public:
    explicit InfoScreen(const DeviceConfig& config);
    void show(DisplayThing& displayThing) override;
private:
    DeviceConfig m_config;
};
