#pragma once

#include <memory>
#include "display/configuration/ConfigurationManager.h"
#include "screens/Screen.h"
#include "time/TimeManager.h"

class DisplayThing;

class DisplayManager {
public:
    explicit DisplayManager(DisplayThing& displayThing, TimeManager& timeManager);
    void showCurrentModule() const;
    void updateCurrentModule() const;
    void goToNextModule(const DeviceConfig& deviceConfig);
    void buildQueue(const DeviceConfig& deviceConfig);
    unsigned int getCurrentModuleDuration(const DeviceConfig& deviceConfig) const;

private:
    std::unique_ptr<Screen> createModule(const std::string& name) const;
    DisplayThing& displayThing;
    TimeManager& m_timeManager;
    std::vector<std::unique_ptr<Screen>> m_module_queue;
    int m_current_module_index = -1;
};
