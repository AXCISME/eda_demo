#include "infrastructure/transport/modbus/LibmodbusMasterTcpAdapter.h"

#include <cerrno>
#include <cstdint>
#include <utility>
#include <vector>

#include "runtime/logging/Logger.h"

namespace
{
std::vector<uint8_t> to_modbus_bits(const std::vector<uint16_t>& values)
{
    std::vector<uint8_t> bits;
    bits.reserve(values.size());

    for (const uint16_t value : values)
    {
        bits.push_back(value == 0 ? 0 : 1);
    }

    return bits;
}

void copy_modbus_bits(const std::vector<uint8_t>& bits, std::vector<uint16_t>& out)
{
    out.clear();
    out.reserve(bits.size());

    for (const uint8_t bit : bits)
    {
        out.push_back(bit == 0 ? 0 : 1);
    }
}
}

LibmodbusMasterTcpAdapter::LibmodbusMasterTcpAdapter(std::string host, int port)
    : host_(std::move(host)),
      port_(port > 0 ? port : 502)
{
}

LibmodbusMasterTcpAdapter::~LibmodbusMasterTcpAdapter()
{
    disconnect();
}

bool LibmodbusMasterTcpAdapter::connect()
{
    if (context_ != nullptr)
    {
        return true;
    }

    context_ = modbus_new_tcp(host_.c_str(), port_);
    if (context_ == nullptr)
    {
        Logger::error("[LibmodbusMasterTcpAdapter] modbus_new_tcp failed");
        return false;
    }

    if (modbus_connect(context_) == -1)
    {
        Logger::error(
            std::string("[LibmodbusMasterTcpAdapter] connect failed: ")
            + modbus_strerror(errno));
        modbus_free(context_);
        context_ = nullptr;
        return false;
    }

    Logger::info(
        "[LibmodbusMasterTcpAdapter] connected host=" + host_
        + " port=" + std::to_string(port_));
    return true;
}

void LibmodbusMasterTcpAdapter::disconnect()
{
    if (context_ == nullptr)
    {
        return;
    }

    modbus_close(context_);
    modbus_free(context_);
    context_ = nullptr;

    Logger::info("[LibmodbusMasterTcpAdapter] disconnected");
}

bool LibmodbusMasterTcpAdapter::read_coils(
    int slave_id,
    int addr,
    int count,
    std::vector<uint16_t>& out)
{
    if (context_ == nullptr || !ensure_slave(slave_id))
    {
        return false;
    }

    std::vector<uint8_t> bits(static_cast<std::size_t>(count), 0);

    const int rc = modbus_read_bits(context_, addr, count, bits.data());
    if (rc != count)
    {
        Logger::error(
            std::string("[LibmodbusMasterTcpAdapter] read_coils failed: ")
            + modbus_strerror(errno));
        out.clear();
        return false;
    }

    copy_modbus_bits(bits, out);
    return true;
}

bool LibmodbusMasterTcpAdapter::read_discrete_inputs(
    int slave_id,
    int addr,
    int count,
    std::vector<uint16_t>& out)
{
    if (context_ == nullptr || !ensure_slave(slave_id))
    {
        return false;
    }

    std::vector<uint8_t> bits(static_cast<std::size_t>(count), 0);

    const int rc = modbus_read_input_bits(context_, addr, count, bits.data());
    if (rc != count)
    {
        Logger::error(
            std::string("[LibmodbusMasterTcpAdapter] read_discrete_inputs failed: ")
            + modbus_strerror(errno));
        out.clear();
        return false;
    }

    copy_modbus_bits(bits, out);
    return true;
}

bool LibmodbusMasterTcpAdapter::read_input_registers(
    int slave_id,
    int addr,
    int count,
    std::vector<uint16_t>& out)
{
    if (context_ == nullptr || !ensure_slave(slave_id))
    {
        return false;
    }

    out.assign(static_cast<std::size_t>(count), 0);

    const int rc = modbus_read_input_registers(context_, addr, count, out.data());
    if (rc != count)
    {
        Logger::error(
            std::string("[LibmodbusMasterTcpAdapter] read_input_registers failed: ")
            + modbus_strerror(errno));
        out.clear();
        return false;
    }

    return true;
}

bool LibmodbusMasterTcpAdapter::read_holding_registers(
    int slave_id,
    int addr,
    int count,
    std::vector<uint16_t>& out)
{
    if (context_ == nullptr || !ensure_slave(slave_id))
    {
        return false;
    }

    out.assign(static_cast<std::size_t>(count), 0);

    const int rc = modbus_read_registers(context_, addr, count, out.data());
    if (rc != count)
    {
        Logger::error(
            std::string("[LibmodbusMasterTcpAdapter] read_holding_registers failed: ")
            + modbus_strerror(errno));
        out.clear();
        return false;
    }

    return true;
}

bool LibmodbusMasterTcpAdapter::write_single_coil(
    int slave_id,
    int addr,
    uint16_t value)
{
    if (context_ == nullptr || !ensure_slave(slave_id))
    {
        return false;
    }

    const int rc = modbus_write_bit(context_, addr, value == 0 ? 0 : 1);
    if (rc == -1)
    {
        Logger::error(
            std::string("[LibmodbusMasterTcpAdapter] write_single_coil failed: ")
            + modbus_strerror(errno));
        return false;
    }

    return true;
}

bool LibmodbusMasterTcpAdapter::write_single_register(
    int slave_id,
    int addr,
    uint16_t value)
{
    if (context_ == nullptr || !ensure_slave(slave_id))
    {
        return false;
    }

    const int rc = modbus_write_register(context_, addr, value);
    if (rc == -1)
    {
        Logger::error(
            std::string("[LibmodbusMasterTcpAdapter] write_single_register failed: ")
            + modbus_strerror(errno));
        return false;
    }

    return true;
}

bool LibmodbusMasterTcpAdapter::write_multiple_coils(
    int slave_id,
    int addr,
    const std::vector<uint16_t>& values)
{
    if (context_ == nullptr || !ensure_slave(slave_id))
    {
        return false;
    }

    std::vector<uint8_t> bits = to_modbus_bits(values);
    const int rc = modbus_write_bits(
        context_,
        addr,
        static_cast<int>(bits.size()),
        bits.data());
    if (rc == -1)
    {
        Logger::error(
            std::string("[LibmodbusMasterTcpAdapter] write_multiple_coils failed: ")
            + modbus_strerror(errno));
        return false;
    }

    return true;
}

bool LibmodbusMasterTcpAdapter::write_multiple_registers(
    int slave_id,
    int addr,
    const std::vector<uint16_t>& values)
{
    if (context_ == nullptr || !ensure_slave(slave_id))
    {
        return false;
    }

    const int rc = modbus_write_registers(
        context_,
        addr,
        static_cast<int>(values.size()),
        values.data());
    if (rc == -1)
    {
        Logger::error(
            std::string("[LibmodbusMasterTcpAdapter] write_multiple_registers failed: ")
            + modbus_strerror(errno));
        return false;
    }

    return true;
}

bool LibmodbusMasterTcpAdapter::ensure_slave(int slave_id)
{
    if (context_ == nullptr)
    {
        return false;
    }

    if (modbus_set_slave(context_, slave_id) == -1)
    {
        Logger::error(
            std::string("[LibmodbusMasterTcpAdapter] modbus_set_slave failed: ")
            + modbus_strerror(errno));
        return false;
    }

    return true;
}
