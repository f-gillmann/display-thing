#include "DisplayThing.h"

GxEPD2_BW<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT>& DisplayThing::getDisplay()
{
    return display_instance;
}

WebServer& DisplayThing::getWebServer()
{
    return web_server_instance;
}

DNSServer& DisplayThing::getDnsServer()
{
    return dns_server_instance;
}

Preferences& DisplayThing::getPreferences()
{
    return preferences_instance;
}
