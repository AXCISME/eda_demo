/**
 * IHttpRouteRegistry.h
 * 定义"怎么往服务里挂路由"，由框架内部集成实现
 */
#pragma once
#include <functional>
#include <string>
#include "infrastructure/transport/http/HttpRequest.h"
#include "infrastructure/transport/http/HttpResponse.h"

class IHttpRouteRegistry
{
public:
    using Handler = std::function<HttpResponse(const HttpRequest&)>;

    virtual ~IHttpRouteRegistry() = default;

    // 虚函数，定义了挂路由的规则，必须由继承者实现
    virtual void add_route(
        const std::string& method,
        const std::string& path,
        Handler handler
    ) = 0;
};
