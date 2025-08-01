#pragma once

#include <memory>
#include "configuration/ConfigurationManager.h"
#include "screens/Screen.h"

class DisplayThing;

class DisplayManager
{
public:
    explicit DisplayManager(DisplayThing& displayThing);
    void setScreen(std::unique_ptr<Screen> newScreen);
    void update(const DeviceConfig& deviceConfig) const;

private:
    DisplayThing& displayThing;
    std::unique_ptr<Screen> currentScreen;
};
