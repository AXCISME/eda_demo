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
        coils_[1][10] = 1;
        discrete_inputs_[1][20] = 1;
        input_registers_[1][200] = 24;
        input_registers_[1][201] = 68;
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

    bool read_coils(
        int slave_id,
        int addr,
        int count,
        std::vector<uint16_t>& out) override
    {
        return read_values(coils_, "read coils", slave_id, addr, count, out);
    }

    bool read_discrete_inputs(
        int slave_id,
        int addr,
        int count,
        std::vector<uint16_t>& out) override
    {
        return read_values(discrete_inputs_, "read discrete inputs", slave_id, addr, count, out);
    }

    bool read_input_registers(
        int slave_id,
        int addr,
        int count,
        std::vector<uint16_t>& out) override
    {
        return read_values(input_registers_, "read input registers", slave_id, addr, count, out);
    }

    bool read_holding_registers(
        int slave_id,
        int addr,
        int count,
        std::vector<uint16_t>& out) override
    {
        return read_values(holding_registers_, "read holding registers", slave_id, addr, count, out);
    }

    bool write_single_coil(
        int slave_id,
        int addr,
        uint16_t value) override
    {
        if (!ensure_connected("write"))
        {
            return false;
        }

        coils_[slave_id][addr] = value == 0 ? 0 : 1;

        Logger::info(
            "[FakeModbusMasterAdapter] write single coil slave=" +
            std::to_string(slave_id) +
            " addr=" + std::to_string(addr) +
            " value=" + std::to_string(coils_[slave_id][addr])
        );
        return true;
    }

    bool write_single_register(
        int slave_id,
        int addr,
        uint16_t value) override
    {
        if (!ensure_connected("write"))
        {
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

    bool write_multiple_coils(
        int slave_id,
        int addr,
        const std::vector<uint16_t>& values) override
    {
        if (!ensure_connected("write"))
        {
            return false;
        }

        for (std::size_t index = 0; index < values.size(); ++index)
        {
            coils_[slave_id][addr + static_cast<int>(index)] =
                values[index] == 0 ? 0 : 1;
        }

        Logger::info(
            "[FakeModbusMasterAdapter] write multiple coils slave=" +
            std::to_string(slave_id) +
            " addr=" + std::to_string(addr) +
            " count=" + std::to_string(values.size())
        );
        return true;
    }

    bool write_multiple_registers(
        int slave_id,
        int addr,
        const std::vector<uint16_t>& values) override
    {
        if (!ensure_connected("write"))
        {
            return false;
        }

        for (std::size_t index = 0; index < values.size(); ++index)
        {
            holding_registers_[slave_id][addr + static_cast<int>(index)] = values[index];
        }

        Logger::info(
            "[FakeModbusMasterAdapter] write multiple registers slave=" +
            std::to_string(slave_id) +
            " addr=" + std::to_string(addr) +
            " count=" + std::to_string(values.size())
        );
        return true;
    }

private:
    using DataTable = std::map<int, std::map<int, uint16_t>>;

    bool ensure_connected(const std::string& operation) const
    {
        if (connected_)
        {
            return true;
        }

        Logger::error("[FakeModbusMasterAdapter] " + operation + " requested while disconnected");
        return false;
    }

    bool read_values(
        DataTable& table,
        const std::string& operation,
        int slave_id,
        int addr,
        int count,
        std::vector<uint16_t>& out)
    {
        if (!ensure_connected("read"))
        {
            return false;
        }

        out.clear();
        for (int offset = 0; offset < count; ++offset)
        {
            out.push_back(table[slave_id][addr + offset]);
        }

        Logger::info(
            "[FakeModbusMasterAdapter] " + operation + " slave=" +
            std::to_string(slave_id) +
            " addr=" + std::to_string(addr) +
            " count=" + std::to_string(count)
        );
        return true;
    }

    bool connected_ {false};
    DataTable coils_;
    DataTable discrete_inputs_;
    DataTable input_registers_;
    DataTable holding_registers_;
};
