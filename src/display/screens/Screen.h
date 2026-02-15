#pragma once

#include "DisplayThing.h"
#include "display/configuration/ConfigurationManager.h"

class Screen
{
public:
    virtual ~Screen() = default;
    virtual void show(DisplayThing& displayThing) = 0;
    virtual void setConfig(const DeviceConfig& deviceConfig) = 0;

    virtual void update()
    {
    }

    virtual void onFirstShow(DisplayThing& displayThing) { show(displayThing); }
    virtual bool needsFrequentUpdates() const { return false; }
    virtual unsigned long getUpdateInterval() const { return 1000; }

protected:
    bool m_isFirstShow = true;
};
