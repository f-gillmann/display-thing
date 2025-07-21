#pragma once

#include <DNSServer.h>
#include <GxEPD2_BW.h>
#include <Preferences.h>
#include <WebServer.h>
#include "config.h"

class DisplayThing
{
public:
    GxEPD2_BW<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT>& getDisplay();
    WebServer& getWebServer();
    DNSServer& getDnsServer();
    Preferences& getPreferences();

private:
    GxEPD2_BW<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT> display_instance{
        GxEPD2_750_GDEY075T7(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
    };
    WebServer web_server_instance{80};
    DNSServer dns_server_instance;
    Preferences preferences_instance;
};
