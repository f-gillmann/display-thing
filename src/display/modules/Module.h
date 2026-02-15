#pragma once
#include "display/screens/Screen.h"

class Module : public Screen
{
public:
    ~Module() override = default;
    void update() override = 0;

    // Called when this module is first displayed (allows full refresh)
    void onFirstShow(DisplayThing& displayThing) override
    {
        drawContent(displayThing, false); // Full refresh on first show
        m_isFirstShow = false;
    }

    // Regular show uses partial updates to avoid black flash
    void show(DisplayThing& displayThing) override
    {
        drawContent(displayThing, !m_isFirstShow); // Partial update after first show
        m_isFirstShow = false;
    }

    // Check if this module needs frequent updates (like clock with seconds)
    bool needsFrequentUpdates() const override { return false; }

    // Get update interval in milliseconds (for modules that need frequent updates)
    unsigned long getUpdateInterval() const override { return 60000; } // Default: 1 minute

protected:
    // Subclasses implement this to draw their content
    virtual void drawContent(DisplayThing& displayThing, bool usePartialUpdate) = 0;
};
