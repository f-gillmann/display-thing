#pragma once
#include "display/screens/Screen.h"

class Module : public Screen
{
public:
    ~Module() override = default;
    void update() override = 0;
};
