#pragma once

#include "DisplayThing.h"

extern DisplayThing displayThing;

class Screen
{
public:
    virtual ~Screen() = default;
    virtual void show(DisplayThing& displayThing) = 0;
};
