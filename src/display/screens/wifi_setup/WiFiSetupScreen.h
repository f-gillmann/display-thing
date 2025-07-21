#pragma once

#include "display/screens/Screen.h"
#include <string>

class WiFiSetupScreen final : public Screen
{
public:
    explicit WiFiSetupScreen(std::string ap_password);
    void show(DisplayThing& displayThing) override;

private:
    std::string access_point_password;
};
