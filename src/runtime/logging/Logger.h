#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <type_traits>
#include "domain/model/EventData.h"

class Logger {
public:
    static void info(const std::string& msg) {
        std::cout << "[INFO] " << msg << std::endl;
    }

    static void warn(const std::string& msg) {
        std::cout << "[WARN] " << msg << std::endl;
    }

    static void error(const std::string& msg) {
        std::cout << "[ERROR] " << msg << std::endl;
    }

    static std::string to_string(const DeviceSample& d) {
        std::ostringstream oss;
        oss << "DeviceSample{device_id=" << d.device_id
            << ", temperature=" << d.temperature
            << ", pressure=" << d.pressure << "}";
        return oss.str();
    }

    static std::string to_string(const ControlCommand& c) {
        std::ostringstream oss;
        oss << "ControlCommand{addr=" << c.addr
            << ", value=" << c.value << "}";
        return oss.str();
    }

    static std::string to_string(const WsClientInfo& c) {
        return "WsClientInfo{client_id=" + c.client_id + "}";
    }

    static std::string to_string(const WsMessage& m) {
        return "WsMessage{client_id=" + m.client_id + ", text=" + m.text + "}";
    }

    static std::string to_string(const EventData& data) {
        return std::visit([](const auto& value) ->std::string {
            using T = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<T, std::monostate>)
                return "{}";
            else if constexpr (std::is_same_v<T, DeviceSample>)
                return Logger::to_string(value);
            else if constexpr (std::is_same_v<T, ControlCommand>)
                return Logger::to_string(value);
            else if constexpr (std::is_same_v<T, WsClientInfo>)
                return Logger::to_string(value);
            else if constexpr (std::is_same_v<T, WsMessage>)
                return Logger::to_string(value);
            else
                return "{unknown}";
        }, data);
    }
};
