#pragma once

#include <DNSServer.h>
#include <GxEPD2_BW.h>
#include <Preferences.h>
#include <WebServer.h>
#include "config.h"

extern GxEPD2_BW<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT> display;
extern WebServer server;
extern DNSServer dnsServer;
extern Preferences preferences;
