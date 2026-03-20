#pragma once
#include <functional>
#include <string>
#include "transport/http/HttpRequest.h"
#include "transport/http/HttpResponse.h"

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
