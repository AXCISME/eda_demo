/**
 * HttpClientModule.h
 * 订阅 HTTP client 请求事件，调用底层 adapter 后再回发响应/失败事件
 */
#pragma once

#include <memory>
#include <utility>

#include "infrastructure/transport/http/IHttpClientAdapter.h"
#include "runtime/bus/EventBus.h"
#include "runtime/logging/Logger.h"

class HttpClientModule
{
public:
    HttpClientModule(EventBus& bus, std::unique_ptr<IHttpClientAdapter> adapter)
        : bus_(bus),
          adapter_(std::move(adapter))
    {
    }

    void init()
    {
        if (!adapter_)
        {
            Logger::error("[HttpClientModule] adapter is null");
            return;
        }

        bus_.subscribe(FrameworkEvents::HTTP_CLIENT_REQUESTED, [this](const HttpClientRequest& request) {
            on_request(request);
        });
    }

    void stop()
    {
        if (adapter_)
        {
            adapter_->stop();
        }
    }

private:
    void on_request(const HttpClientRequest& request)
    {
        if (!adapter_)
        {
            Logger::error("[HttpClientModule] adapter is null");
            return;
        }

        auto* bus = &bus_;

        adapter_->async_request(
            request,
            [bus](HttpClientResponse response) {
                bus->publish(
                    FrameworkEvents::HTTP_CLIENT_RESPONSE_RECEIVED,
                    "HttpClientModule",
                    std::move(response));
            },
            [bus](HttpClientFailure failure) {
                bus->publish(
                    FrameworkEvents::HTTP_CLIENT_REQUEST_FAILED,
                    "HttpClientModule",
                    std::move(failure));
            });
    }

private:
    EventBus& bus_;
    std::unique_ptr<IHttpClientAdapter> adapter_;
};
