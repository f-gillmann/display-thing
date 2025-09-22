#pragma once

#include "display/screens/Screen.h"
#include <string>

class WiFiReconnectScreen final : public Screen
{
public:
    WiFiReconnectScreen(std::string ap_ssid, std::string ap_password);
    void show(DisplayThing& displayThing) override;
    void setConfig(const DeviceConfig& deviceConfig) override;

private:
    std::string access_point_ssid;
    std::string access_point_password;
};