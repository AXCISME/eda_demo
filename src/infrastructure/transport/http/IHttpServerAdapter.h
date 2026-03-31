/**
 * IHttpServerAdapter.h
 * 适配器的抽象类，继承者必须实现启动、停止、路由注册
 */
#pragma once
#include <functional>
#include <string>
#include "infrastructure/transport/http/HttpRequest.h"
#include "infrastructure/transport/http/HttpResponse.h"

class IHttpServerAdapter
{
public:
    using Handler = std::function<HttpResponse(const HttpRequest&)>;

    virtual ~IHttpServerAdapter() = default;

    virtual bool start(const std::string& host, int port) = 0;
    virtual void stop() = 0;

    virtual void register_handler(
        const std::string& method,
        const std::string& path,
        Handler handler
    ) = 0;
};
