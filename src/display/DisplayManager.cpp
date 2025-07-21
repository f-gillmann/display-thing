#include "DisplayManager.h"
#include "../DisplayThing.h"


DisplayManager::DisplayManager(DisplayThing& displayThing) : displayThing(displayThing)
{
}

void DisplayManager::setScreen(std::unique_ptr<Screen> newScreen)
{
    currentScreen = std::move(newScreen);
    if (currentScreen)
    {
        currentScreen->show(displayThing);
    }
}

void DisplayManager::update() const
{
    if (currentScreen)
    {
        currentScreen->show(displayThing);
    }
}
