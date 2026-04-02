#include "infrastructure/transport/modbus/LibmodbusMasterRtuAdapter.h"

#include <cerrno>
#include <utility>

#include "runtime/logging/Logger.h"

LibmodbusMasterRtuAdapter::LibmodbusMasterRtuAdapter(
    std::string serial_device,
    int baudrate,
    char parity,
    int data_bits,
    int stop_bits)
    : serial_device_(std::move(serial_device)),
      baudrate_(baudrate > 0 ? baudrate : 9600),
      parity_(parity),
      data_bits_(data_bits > 0 ? data_bits : 8),
      stop_bits_(stop_bits > 0 ? stop_bits : 1)
{
}

LibmodbusMasterRtuAdapter::~LibmodbusMasterRtuAdapter()
{
    disconnect();
}

bool LibmodbusMasterRtuAdapter::connect()
{
    if (context_ != nullptr)
    {
        return true;
    }

    context_ = modbus_new_rtu(
        serial_device_.c_str(),
        baudrate_,
        parity_,
        data_bits_,
        stop_bits_);
    if (context_ == nullptr)
    {
        Logger::error("[LibmodbusMasterRtuAdapter] modbus_new_rtu failed");
        return false;
    }

    if (modbus_connect(context_) == -1)
    {
        Logger::error(
            std::string("[LibmodbusMasterRtuAdapter] connect failed: ")
            + modbus_strerror(errno));
        modbus_free(context_);
        context_ = nullptr;
        return false;
    }

    Logger::info(
        "[LibmodbusMasterRtuAdapter] connected device=" + serial_device_
        + " baudrate=" + std::to_string(baudrate_));
    return true;
}

void LibmodbusMasterRtuAdapter::disconnect()
{
    if (context_ == nullptr)
    {
        return;
    }

    modbus_close(context_);
    modbus_free(context_);
    context_ = nullptr;

    Logger::info("[LibmodbusMasterRtuAdapter] disconnected");
}

bool LibmodbusMasterRtuAdapter::read_holding_registers(
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
            std::string("[LibmodbusMasterRtuAdapter] read_holding_registers failed: ")
            + modbus_strerror(errno));
        out.clear();
        return false;
    }

    return true;
}

bool LibmodbusMasterRtuAdapter::write_single_register(
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
            std::string("[LibmodbusMasterRtuAdapter] write_single_register failed: ")
            + modbus_strerror(errno));
        return false;
    }

    return true;
}

bool LibmodbusMasterRtuAdapter::ensure_slave(int slave_id)
{
    if (context_ == nullptr)
    {
        return false;
    }

    if (modbus_set_slave(context_, slave_id) == -1)
    {
        Logger::error(
            std::string("[LibmodbusMasterRtuAdapter] modbus_set_slave failed: ")
            + modbus_strerror(errno));
        return false;
    }

    return true;
}
