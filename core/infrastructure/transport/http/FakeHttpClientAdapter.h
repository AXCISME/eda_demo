#pragma once

#include <utility>

#include "infrastructure/transport/http/IHttpClientAdapter.h"
#include "runtime/logging/Logger.h"

class FakeHttpClientAdapter : public IHttpClientAdapter
{
public:
    void async_request(
        HttpClientRequest request,
        SuccessHandler on_success,
        FailureHandler on_failure) override
    {
        Logger::info(
            "[FakeHttpClientAdapter] request: "
            + request.method + " " + request.url
            + " request_id=" + request.request_id);

        if (request.url.empty())
        {
            if (on_failure)
            {
                HttpClientFailure failure;
                failure.request_id = request.request_id;
                failure.error_message = "url is empty";
                on_failure(std::move(failure));
            }
            return;
        }

        if (request.url.find("fail") != std::string::npos)
        {
            if (on_failure)
            {
                HttpClientFailure failure;
                failure.request_id = request.request_id;
                failure.error_message = "simulated client failure";
                on_failure(std::move(failure));
            }
            return;
        }

        if (on_success)
        {
            HttpClientResponse response;
            response.request_id = request.request_id;
            response.status = 200;
            response.body =
                "{\"ok\":true,\"method\":\"" + request.method
                + "\",\"url\":\"" + request.url + "\"}";
            response.headers["Content-Type"] = "application/json";
            on_success(std::move(response));
        }
    }
};
