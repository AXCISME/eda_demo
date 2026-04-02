#pragma once

#include "runtime/bus/EventBus.h"

class IBusinessModule
{
public:
    explicit IBusinessModule(EventBus& bus)
        : bus_(bus)
    {
    }

    virtual ~IBusinessModule() = default;

    virtual void install() = 0;

protected:
    EventBus& bus_;
};
