#pragma once

#include <string>
#include "DisplayThing.h"

class WiFiSetupManager
{
public:
    explicit WiFiSetupManager(DisplayThing& displayThing);
    std::string getAPPassword() const;
    std::string getAPSsid() const;
    void startAP();
    bool connect();
    void handleClient() const;
    bool manageConnection();

private:
    bool attemptConnection() const;
    bool hasStoredCredentials() const;
    DisplayThing& displayThing;
    std::string access_point_password;
    std::string access_point_ssid;
    unsigned long lastReconnectAttempt = 0;
    int reconnectAttempts = 0;
    bool apModeStarted = false;
    static constexpr long reconnectInterval = 10000;
    static constexpr int maxReconnectAttempts = 3;
};