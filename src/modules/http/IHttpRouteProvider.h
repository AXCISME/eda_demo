#pragma once
#include "modules/http/IHttpRouteRegistry.h"

class IHttpRouteProvider
{
public:
    virtual ~IHttpRouteProvider() = default;
    virtual void register_routes(IHttpRouteRegistry& registry) = 0;
};
