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

private:
    void startAP();

    DisplayThing& displayThing;
    std::string ap_password;
};
