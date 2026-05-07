#pragma once
#include <cstdint>
#include <vector>

class IModbusMasterAdapter
{
public:
    virtual ~IModbusMasterAdapter() = default;

    virtual bool connect() = 0;
    virtual void disconnect() = 0;

    virtual bool read_coils(int slave, int addr, int count, std::vector<uint16_t>& out) = 0;
    virtual bool read_discrete_inputs(int slave, int addr, int count, std::vector<uint16_t>& out) = 0;
    virtual bool read_input_registers(int slave, int addr, int count, std::vector<uint16_t>& out) = 0;
    virtual bool read_holding_registers(int slave, int addr, int count, std::vector<uint16_t>& out) = 0;
    virtual bool write_single_coil(int slave, int addr, uint16_t value) = 0;
    virtual bool write_single_register(int slave, int addr, uint16_t value) = 0;
    virtual bool write_multiple_coils(int slave, int addr, const std::vector<uint16_t>& values) = 0;
    virtual bool write_multiple_registers(int slave, int addr, const std::vector<uint16_t>& values) = 0;
};
