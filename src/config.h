#pragma once

// Pins
#define EPD_DIN   14
#define EPD_CLK   13
#define EPD_CS    15
#define EPD_DC    27
#define EPD_RST   26
#define EPD_BUSY  25

#define EPD_MOSI  EPD_DIN
#define EPD_MISO  (-1)
#define EPD_SCK   EPD_CLK

// Wifi
const IPAddress ACCESS_POINT_IP(10, 10, 10, 10);

// Preferences namespaces
constexpr auto PREFERENCES_WIFI_CONFIG = "wifi_config";
constexpr auto PREFERENCES_DEVICE_CONFIG = "device_config";