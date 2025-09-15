#include "TimeManager.h"
#include "display/configuration/ConfigurationManager.h"

TimeManager::TimeManager(NTPClient& time_client, ConfigurationManager& config_manager)
    : m_timeClient(time_client), m_configManager(config_manager)
{
}

void TimeManager::begin() const
{
    m_configManager.onConfigChanged([this](const DeviceConfig& config) {
        this->handleConfigChange(config);
    });

    const DeviceConfig& currentConfig = m_configManager.getConfig();
    handleConfigChange(currentConfig);

    m_timeClient.begin();
}

NTPClient& TimeManager::getTimeClient() const
{
    return m_timeClient;
}

void TimeManager::handleConfigChange(const DeviceConfig& config) const
{
    Serial.printf("[TimeManager] Applying time offset: %ud seconds\n", config.time_offset);
    m_timeClient.setTimeOffset(config.time_offset);
}
