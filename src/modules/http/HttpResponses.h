#pragma once
#include <string>
#include "transport/http/HttpResponse.h"

inline HttpResponse make_json_response(int status, std::string body)
{
    HttpResponse resp;
    resp.status = status;
    resp.body = std::move(body);
    resp.headers["Content-Type"] = "application/json";
    return resp;
}
