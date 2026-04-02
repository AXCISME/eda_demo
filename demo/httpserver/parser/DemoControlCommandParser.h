#pragma once
#include <string>
#include "domain/model/EventData.h"
#include "infrastructure/transport/http/HttpRequest.h"

namespace demo_http
{
inline bool parse_control_command(const HttpRequest& req, ControlCommand& out)
{
    const std::string& body = req.body;
    auto addr_pos = body.find("addr=");
    auto value_pos = body.find("value=");

    if (addr_pos == std::string::npos || value_pos == std::string::npos)
    {
        return false;
    }

    try
    {
        std::size_t addr_end = body.find('&', addr_pos);
        std::string addr_str = body.substr(
            addr_pos + 5,
            addr_end == std::string::npos ? std::string::npos : addr_end - (addr_pos + 5)
        );

        std::string value_str = body.substr(value_pos + 6);

        out.addr = std::stoi(addr_str);
        out.value = std::stoi(value_str);
    }
    catch (...)
    {
        return false;
    }

    return true;
}
}
