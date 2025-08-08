#pragma once

#include "DisplayThing.h"
#include <string>

class WiFiSetupManager
{
public:
    explicit WiFiSetupManager(DisplayThing& displayThing);
    bool connect();
    void handleClient() const;
    std::string getAPPassword() const;
    std::string getAPSsid() const;

private:
    void startAP();

    DisplayThing& displayThing;
    std::string access_point_password;
    std::string access_point_ssid;
};
