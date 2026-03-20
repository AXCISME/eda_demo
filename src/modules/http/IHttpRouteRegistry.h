// src/modules/http/IHttpRouteRegistry.h
#pragma once
#include <functional>
#include <string>
#include "transport/http/HttpRequest.h"
#include "transport/http/HttpResponse.h"

class IHttpRouteRegistry
{
public:
    using Handler = std::function<HttpResponse(const HttpRequest&)>;

    virtual ~IHttpRouteRegistry() = default;

    virtual void add_route(
        const std::string& method,
        const std::string& path,
        Handler handler
    ) = 0;
};
