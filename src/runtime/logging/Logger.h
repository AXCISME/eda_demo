#pragma once
#include <iostream>
#include <sstream>
#include <string>

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
        return format_event_payload(data);
    }
};
