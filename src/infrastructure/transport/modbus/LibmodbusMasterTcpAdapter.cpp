#include "infrastructure/transport/modbus/LibmodbusMasterTcpAdapter.h"

#include <cerrno>
#include <utility>

#include "runtime/logging/Logger.h"

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
