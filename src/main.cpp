#include <Arduino.h>
#include <GxEPD2_BW.h>
#include "DNSServer.h"
#include "Preferences.h"
#include "WebServer.h"
#include "wifi_setup.h"

#pragma region EPD PINS
#define EPD_DIN   4
#define EPD_CLK   5
#define EPD_CS    18
#define EPD_DC    19
#define EPD_RST   21
#define EPD_BUSY  22
#define EPD_PWR   23

#define EPD_MOSI  EPD_DIN
#define EPD_MISO  (-1)
#define EPD_SCK   EPD_CLK
#pragma endregion

WebServer server(80);
DNSServer dnsServer;
Preferences preferences;
bool isConnected = false;

GxEPD2_BW<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT> display(
    GxEPD2_750_GDEY075T7(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
);

void setup()
{
    pinMode(EPD_PWR, OUTPUT);
    digitalWrite(EPD_PWR, HIGH);

    Serial.begin(115200);
    Serial.println("Setup starting...");

    SPI.begin(EPD_SCK, EPD_MISO, EPD_MOSI);
    display.init(115200, true, 2, false);
    display.setRotation(2);
    isConnected = setup_wifi();

    if (isConnected)
    {
        Serial.println("Connected.");
        Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
    }
    else
    {
        Serial.println("Not connected, configure in your browser.");
    }

    Serial.println("Setup finished.");
}

void loop()
{
    if (isConnected)
    {

    }
    else
    {
        loop_wifi();
    }
}
