#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include "runtime/logging/Logger.h"
#include "infrastructure/transport/modbus/IModbusMasterAdapter.h"

class FakeModbusMasterAdapter : public IModbusMasterAdapter
{
public:
    FakeModbusMasterAdapter()
    {
        holding_registers_[1][100] = 365;
        holding_registers_[1][101] = 1012;
    }

    bool connect() override
    {
        if (connected_)
        {
            Logger::warn("[FakeModbusMasterAdapter] already connected");
            return true;
        }

        connected_ = true;
        Logger::info("[FakeModbusMasterAdapter] connected");
        return true;
    }

    void disconnect() override
    {
        if (!connected_)
        {
            return;
        }

        connected_ = false;
        Logger::info("[FakeModbusMasterAdapter] disconnected");
    }

    bool read_holding_registers(
        int slave_id,
        int addr,
        int count,
        std::vector<uint16_t>& out) override
    {
        if (!connected_)
        {
            Logger::error("[FakeModbusMasterAdapter] read requested while disconnected");
            return false;
        }

        out.clear();
        for (int offset = 0; offset < count; ++offset)
        {
            out.push_back(holding_registers_[slave_id][addr + offset]);
        }

        Logger::info(
            "[FakeModbusMasterAdapter] read holding registers slave=" +
            std::to_string(slave_id) +
            " addr=" + std::to_string(addr) +
            " count=" + std::to_string(count)
        );
        return true;
    }

    bool write_single_register(
        int slave_id,
        int addr,
        uint16_t value) override
    {
        if (!connected_)
        {
            Logger::error("[FakeModbusMasterAdapter] write requested while disconnected");
            return false;
        }

        holding_registers_[slave_id][addr] = value;

        Logger::info(
            "[FakeModbusMasterAdapter] write single register slave=" +
            std::to_string(slave_id) +
            " addr=" + std::to_string(addr) +
            " value=" + std::to_string(value)
        );
        return true;
    }

private:
    bool connected_ {false};
    std::map<int, std::map<int, uint16_t>> holding_registers_;
};
