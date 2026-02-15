#pragma once

#include <Adafruit_GFX.h>

class DisplayThing;

void drawCenteredString(DisplayThing& displayThing, const char* text, const GFXfont* font, int x, int y,
                        bool centerX = true, bool centerY = false);
