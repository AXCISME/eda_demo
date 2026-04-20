#pragma once

#include <utility>

#include "application/modules/IBusinessModule.h"
#include "demo/timer_test/TimerDemoEvents.h"
#include "domain/events/DomainEvent.h"
#include "runtime/logging/Logger.h"

class UpdateModule : public IBusinessModule {
public:
    explicit UpdateModule(EventBus& bus)
        : IBusinessModule(bus)
    {
    }

    void install() override {
        bus_.subscribe(TimerDemoEvents::TICK_5S, [this](const TimerDemoEvents::TimerTickPayload& payload) {
            update_data1(payload);
        });

        bus_.subscribe(TimerDemoEvents::TICK_10S, [this](const TimerDemoEvents::TimerTickPayload& payload) {
            update_data2(payload);
        });

        bus_.subscribe(FrameworkEvents::HTTP_CLIENT_RESPONSE_RECEIVED, [this](const HttpClientResponse& response) {
            on_http_response(response);
        });

        bus_.subscribe(FrameworkEvents::HTTP_CLIENT_REQUEST_FAILED, [this](const HttpClientFailure& failure) {
            on_http_failure(failure);
        });
    }

    void update_data1(const TimerDemoEvents::TimerTickPayload& payload) {
        auto request = make_request(
            "update_data1",
            payload,
            "GET",
            "http://www.baidu.com");

        Logger::info(
            "[UpdateModule] publish HTTP request for update_data1: request_id="
            + request.request_id);

        bus_.publish(
            FrameworkEvents::HTTP_CLIENT_REQUESTED,
            "UpdateModule",
            std::move(request));
    }

    void update_data2(const TimerDemoEvents::TimerTickPayload& payload) {
        auto request = make_request(
            "update_data2",
            payload,
            "POST",
            "http://fake.local/update/data2");

        request.body =
            "{\"timer_name\":\"" + payload.timer_name
            + "\",\"interval_ms\":" + std::to_string(payload.interval_ms)
            + "}";
        request.headers["Content-Type"] = "application/json";

        Logger::info(
            "[UpdateModule] publish HTTP request for update_data2: request_id="
            + request.request_id);

        bus_.publish(
            FrameworkEvents::HTTP_CLIENT_REQUESTED,
            "UpdateModule",
            std::move(request));
    }

private:
    HttpClientRequest make_request(
        const std::string& action,
        const TimerDemoEvents::TimerTickPayload& payload,
        std::string method,
        std::string url) const
    {
        HttpClientRequest request;
        request.request_id =
            action + "." + payload.timer_name + "." + std::to_string(payload.interval_ms);
        request.method = std::move(method);
        request.url = std::move(url);
        request.timeout_ms = 2000;
        request.headers["X-Timer-Name"] = payload.timer_name;
        request.headers["X-Timer-Interval-Ms"] = std::to_string(payload.interval_ms);
        return request;
    }

    void on_http_response(const HttpClientResponse& response)
    {
        if (!is_own_request(response.request_id))
        {
            return;
        }

        Logger::info(
            "[UpdateModule] HTTP response received: request_id="
            + response.request_id
            + " status=" + std::to_string(response.status)
            + " body=" + response.body);
    }

    void on_http_failure(const HttpClientFailure& failure)
    {
        if (!is_own_request(failure.request_id))
        {
            return;
        }

        Logger::error(
            "[UpdateModule] HTTP request failed: request_id="
            + failure.request_id
            + " error=" + failure.error_message);
    }

    bool is_own_request(const std::string& request_id) const
    {
        return request_id.rfind("update_data1.", 0) == 0
            || request_id.rfind("update_data2.", 0) == 0;
    }
};
