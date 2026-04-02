/**
 * IHttpClientAdapter.h
 * HTTP client 适配器抽象，负责异步发起 HTTP 请求
 */
#pragma once

#include <functional>

#include "domain/model/EventData.h"

class IHttpClientAdapter
{
public:
    using SuccessHandler = std::function<void(HttpClientResponse)>;
    using FailureHandler = std::function<void(HttpClientFailure)>;

    virtual ~IHttpClientAdapter() = default;

    virtual void async_request(
        HttpClientRequest request,
        SuccessHandler on_success,
        FailureHandler on_failure) = 0;

    virtual void stop() {}
};
