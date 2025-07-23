#pragma once

// Pins
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

// Wifi
const IPAddress ACCESS_POINT_IP(10, 10, 10, 10);

// Preferences namespaces
constexpr auto PREFERENCES_WIFI_CONFIG = "wifi_config";
constexpr auto PREFERENCES_DEVICE_CONFIG = "device_config";
