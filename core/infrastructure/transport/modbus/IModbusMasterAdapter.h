#pragma once
#include <cstdint>
#include <vector>

class IModbusMasterAdapter
{
public:
    virtual ~IModbusMasterAdapter() = default;

    virtual bool connect() = 0;
    virtual void disconnect() = 0;

    virtual bool read_holding_registers(
        int slave_id,
        int addr,
        int count,
        std::vector<uint16_t>& out) = 0;

    virtual bool write_single_register(
        int slave_id,
        int addr,
        uint16_t value) = 0;
};
