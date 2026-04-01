#pragma once

#include "domain/events/DomainEvent.h"

namespace ApplicationEvents
{
inline const TypedEvent<DeviceSample> TELEMETRY_UPDATED {
    make_event_type(EventCategory::BUSINESS, "APPLICATION.TELEMETRY_UPDATED")
};

inline const TypedEvent<ControlCommand> CONTROL_COMMAND {
    make_event_type(EventCategory::BUSINESS, "APPLICATION.CONTROL_COMMAND")
};
}
