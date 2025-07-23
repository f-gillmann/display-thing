#pragma once

#include "display/screens/Screen.h"
#include <string>

class WiFiSetupScreen final : public Screen
{
public:
    WiFiSetupScreen(std::string ap_ssid, std::string ap_password);
    void show(DisplayThing& displayThing) override;
    void setConfig(const DeviceConfig& deviceConfig) override;

private:
    std::string access_point_ssid;
    std::string access_point_password;
};
