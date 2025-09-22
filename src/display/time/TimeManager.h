#pragma once

#include "NTPClient.h"

class ConfigurationManager;
struct DeviceConfig;

class TimeManager
{
public:
    TimeManager(NTPClient& time_client, ConfigurationManager& config_manager);
    void begin() const;
    void end() const;
    NTPClient& getTimeClient() const;

private:
    void handleConfigChange(const DeviceConfig& config) const;

    NTPClient& m_timeClient;
    ConfigurationManager& m_configManager;
};
