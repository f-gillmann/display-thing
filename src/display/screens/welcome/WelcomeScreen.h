#pragma once

#include "display/screens/Screen.h"

class WelcomeScreen final : public Screen
{
public:
    explicit WelcomeScreen();
    void show(DisplayThing& displayThing) override;
    void setConfig(const DeviceConfig& deviceConfig) override;
};
