#include "infrastructure/runtime/ModbusDeviceRuntime.h"

#include <chrono>
#include <utility>

#include "domain/events/DomainEvent.h"
#include "runtime/logging/Logger.h"

namespace
{
constexpr int kIdleWaitMs = 100;

bool is_writable_register_type(ModbusRegisterType type)
{
    return type == ModbusRegisterType::COIL
        || type == ModbusRegisterType::HOLDING_REGISTER;
}
}

ModbusDeviceRuntime::ModbusDeviceRuntime(
    EventBus& bus,
    std::unique_ptr<IModbusMasterAdapter> adapter,
    const ModbusDeviceConfig& config)
    : bus_(bus),
      adapter_(std::move(adapter)),
      config_(config)
{
}

ModbusDeviceRuntime::~ModbusDeviceRuntime()
{
    stop();
}

bool ModbusDeviceRuntime::start()
{
    if (!adapter_)
    {
        Logger::error("[ModbusDeviceRuntime] adapter is null device=" + config_.device_name);
        return false;
    }

    if (running_.exchange(true))
    {
        Logger::warn("[ModbusDeviceRuntime] already running device=" + config_.device_name);
        return true;
    }

    worker_ = std::thread(&ModbusDeviceRuntime::worker_loop, this);
    return true;
}

void ModbusDeviceRuntime::stop()
{
    if (!running_.exchange(false))
    {
        return;
    }

    cv_.notify_all();
    if (worker_.joinable())
    {
        worker_.join();
    }
}

void ModbusDeviceRuntime::submit_read(ModbusReadRequest req)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pending_ops_.push_back(std::move(req));
    }
    cv_.notify_one();
}

void ModbusDeviceRuntime::submit_write(ModbusWriteRequest req)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pending_ops_.push_back(std::move(req));
    }
    cv_.notify_one();
}

const std::string& ModbusDeviceRuntime::device_name() const
{
    return config_.device_name;
}

void ModbusDeviceRuntime::worker_loop()
{
    Logger::info("[ModbusDeviceRuntime] worker started device=" + config_.device_name);

    while (running_)
    {
        PendingOperation op;
        bool has_op = false;

        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait_for(lock, std::chrono::milliseconds(kIdleWaitMs), [this]() {
                return !running_ || !pending_ops_.empty();
            });

            if (!running_)
            {
                break;
            }

            if (!pending_ops_.empty())
            {
                op = std::move(pending_ops_.front());
                pending_ops_.pop_front();
                has_op = true;
            }
        }

        if (!has_op)
        {
            continue;
        }

        bool ok = false;
        if (const auto* read_req = std::get_if<ModbusReadRequest>(&op))
        {
            ok = perform_read(*read_req);
        }
        else if (const auto* write_req = std::get_if<ModbusWriteRequest>(&op))
        {
            ok = perform_write(*write_req);
        }

        if (!ok)
        {
            mark_disconnected();
            wait_retry_delay();
        }
    }

    mark_disconnected();
    Logger::info("[ModbusDeviceRuntime] worker stopped device=" + config_.device_name);
}

bool ModbusDeviceRuntime::ensure_connected()
{
    if (connected_)
    {
        return true;
    }

    Logger::info("[ModbusDeviceRuntime] connecting adapter device=" + config_.device_name);
    connected_ = adapter_->connect();
    if (!connected_)
    {
        Logger::error("[ModbusDeviceRuntime] connect failed device=" + config_.device_name);
    }

    return connected_;
}

void ModbusDeviceRuntime::mark_disconnected()
{
    if (!connected_)
    {
        return;
    }

    adapter_->disconnect();
    connected_ = false;
}

void ModbusDeviceRuntime::wait_retry_delay()
{
    const int delay_ms = config_.retry_delay_ms > 0 ? config_.retry_delay_ms : 500;

    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait_for(lock, std::chrono::milliseconds(delay_ms), [this]() {
        return !running_;
    });
}

bool ModbusDeviceRuntime::perform_read(const ModbusReadRequest& req)
{
    if (!ensure_connected())
    {
        publish_failure(req.request_id, "connect failed");
        return false;
    }

    std::vector<uint16_t> values;
    if (!do_read(req.reg_type, req.slave_id, req.addr, req.count, values))
    {
        publish_failure(req.request_id, "read failed");
        return false;
    }

    ModbusReadResult result;
    result.request_id = req.request_id;
    result.device_name = config_.device_name;
    result.reg_type = req.reg_type;
    result.slave_id = req.slave_id;
    result.addr = req.addr;
    result.values = std::move(values);

    bus_.publish(FrameworkEvents::READ_RESULT, "ModbusDeviceRuntime", std::move(result));
    return true;
}

bool ModbusDeviceRuntime::perform_write(const ModbusWriteRequest& req)
{
    if (!ensure_connected())
    {
        publish_failure(req.request_id, "connect failed");
        return false;
    }

    if (!is_writable_register_type(req.reg_type))
    {
        publish_failure(req.request_id, "register type is read-only");
        return false;
    }

    if (!do_write(req))
    {
        publish_failure(req.request_id, "write failed");
        return false;
    }

    ModbusWriteResult result;
    result.request_id = req.request_id;
    result.device_name = config_.device_name;
    result.slave_id = req.slave_id;
    result.addr = req.addr;
    result.count = static_cast<int>(req.values.size());

    bus_.publish(FrameworkEvents::WRITE_RESULT, "ModbusDeviceRuntime", std::move(result));
    return true;
}

void ModbusDeviceRuntime::publish_failure(std::string request_id, std::string error_message)
{
    ModbusOperationFailed failure;
    failure.request_id = std::move(request_id);
    failure.device_name = config_.device_name;
    failure.error_message = std::move(error_message);

    bus_.publish(FrameworkEvents::OP_FAILED, "ModbusDeviceRuntime", std::move(failure));
}

bool ModbusDeviceRuntime::do_read(
    ModbusRegisterType type,
    int slave,
    int addr,
    int count,
    std::vector<uint16_t>& out)
{
    switch (type)
    {
        case ModbusRegisterType::COIL:
            return adapter_->read_coils(slave, addr, count, out);
        case ModbusRegisterType::DISCRETE_INPUT:
            return adapter_->read_discrete_inputs(slave, addr, count, out);
        case ModbusRegisterType::INPUT_REGISTER:
            return adapter_->read_input_registers(slave, addr, count, out);
        case ModbusRegisterType::HOLDING_REGISTER:
            return adapter_->read_holding_registers(slave, addr, count, out);
        default:
            return false;
    }
}

bool ModbusDeviceRuntime::do_write(const ModbusWriteRequest& req)
{
    if (req.values.empty())
    {
        return false;
    }

    switch (req.reg_type)
    {
        case ModbusRegisterType::COIL:
            if (req.values.size() == 1)
            {
                return adapter_->write_single_coil(req.slave_id, req.addr, req.values.front());
            }
            return adapter_->write_multiple_coils(req.slave_id, req.addr, req.values);

        case ModbusRegisterType::HOLDING_REGISTER:
            if (req.values.size() == 1)
            {
                return adapter_->write_single_register(req.slave_id, req.addr, req.values.front());
            }
            return adapter_->write_multiple_registers(req.slave_id, req.addr, req.values);

        default:
            return false;
    }
}
