#include "ModuleHelpers.h"
#include "../../DisplayThing.h"

void drawCenteredString(DisplayThing& displayThing, const char* text, const GFXfont* font, const int x, const int y,
                        const bool centerX, const bool centerY)
{
    auto& display = displayThing.getDisplay();
    int16_t x1, y1;
    uint16_t w, h;

    display.setFont(font);
    display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

    int drawX = x;
    int drawY = y;
    if (centerX)
    {
        drawX = (display.width() - w) / 2;
    }
    if (centerY)
    {
        drawY = (display.height() + h) / 2;
    }
    display.setCursor(static_cast<int16_t>(drawX), static_cast<int16_t>(drawY));
    display.print(text);
}
