#include "DisplayManager.h"
#include "util.hpp"
#include "modules/clock/ClockModule.h"
#include "modules/weather/WeatherModule.h"
#include "screens/welcome/WelcomeScreen.h"

DisplayManager::DisplayManager(DisplayThing& displayThing, TimeManager& timeManager)
    : displayThing(displayThing), m_timeManager(timeManager)
{
}

void DisplayManager::buildQueue(const DeviceConfig& deviceConfig)
{
    m_module_queue.clear();
    m_current_module_index = -1;

    for (const auto& item : deviceConfig.queue)
    {
        auto module = createModule(item.name.c_str());
        if (module)
        {
            module->setConfig(deviceConfig);
            m_module_queue.push_back(std::move(module));
        }
    }

    if (!m_module_queue.empty())
    {
        m_current_module_index = 0;
    }
    else
    {
        auto welcomeScreen = make_unique<WelcomeScreen>();
        welcomeScreen->setConfig(deviceConfig);
        m_module_queue.push_back(std::move(welcomeScreen));
        m_current_module_index = 0;
    }
}

void DisplayManager::showCurrentModule() const
{
    if (m_current_module_index != -1 && !m_module_queue.empty())
    {
        m_module_queue[m_current_module_index]->show(displayThing);
    }
}

void DisplayManager::updateCurrentModule() const
{
    if (m_current_module_index != -1 && !m_module_queue.empty())
    {
        m_module_queue[m_current_module_index]->update();
    }
}

void DisplayManager::goToNextModule(const DeviceConfig& deviceConfig)
{
    if (!deviceConfig.queue.empty() && deviceConfig.queue.size() > 1)
    {
        m_current_module_index++;
        if (m_current_module_index >= m_module_queue.size())
        {
            m_current_module_index = 0;
        }
    }
}

unsigned int DisplayManager::getCurrentModuleDuration(const DeviceConfig& deviceConfig) const
{
    if (deviceConfig.queue.empty())
    {
        return 60000;
    }

    if (m_current_module_index != -1 && m_current_module_index < deviceConfig.queue.size())
    {
        return deviceConfig.queue[m_current_module_index].duration * 1000;
    }

    return 60000;
}

std::unique_ptr<Screen> DisplayManager::createModule(const std::string& name) const
{
    if (name == "weather")
        return make_unique<WeatherModule>();
    if (name == "clock")
            return make_unique<ClockModule>(m_timeManager);

    return nullptr;
}
