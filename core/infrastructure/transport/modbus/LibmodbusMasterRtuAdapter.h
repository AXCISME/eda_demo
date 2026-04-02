#pragma once

#include <string>
#include <vector>

#include "infrastructure/transport/modbus/IModbusMasterAdapter.h"

#if __has_include(<modbus/modbus.h>)
#include <modbus/modbus.h>
#elif __has_include(<modbus.h>)
#include <modbus.h>
#else
#error "libmodbus header not found"
#endif

class LibmodbusMasterRtuAdapter : public IModbusMasterAdapter
{
public:
    LibmodbusMasterRtuAdapter(
        std::string serial_device,
        int baudrate,
        char parity,
        int data_bits,
        int stop_bits);
    ~LibmodbusMasterRtuAdapter() override;

    bool connect() override;
    void disconnect() override;

    bool read_holding_registers(
        int slave_id,
        int addr,
        int count,
        std::vector<uint16_t>& out) override;

    bool write_single_register(
        int slave_id,
        int addr,
        uint16_t value) override;

private:
    bool ensure_slave(int slave_id);

private:
    std::string serial_device_;
    int baudrate_ {9600};
    char parity_ {'N'};
    int data_bits_ {8};
    int stop_bits_ {1};
    modbus_t* context_ {nullptr};
};
