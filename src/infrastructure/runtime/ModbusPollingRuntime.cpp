#include "infrastructure/runtime/ModbusPollingRuntime.h"

#include <chrono>
#include <utility>
#include <vector>

#include "runtime/logging/Logger.h"

ModbusPollingRuntime::ModbusPollingRuntime(
    EventBus& bus,
    std::unique_ptr<IModbusMasterAdapter> adapter,
    int poll_interval_ms,
    int slave_id,
    int sample_base_addr)
    : bus_(bus),
      adapter_(std::move(adapter)),
      poll_interval_ms_(poll_interval_ms > 0 ? poll_interval_ms : 1000),
      slave_id_(slave_id),
      sample_base_addr_(sample_base_addr)
{
}

ModbusPollingRuntime::~ModbusPollingRuntime()
{
    stop();
}

bool ModbusPollingRuntime::start()
{
    if (!adapter_)
    {
        Logger::error("[ModbusPollingRuntime] adapter is null");
        return false;
    }

    if (running_.exchange(true))
    {
        Logger::warn("[ModbusPollingRuntime] already running");
        return true;
    }

    worker_ = std::thread(&ModbusPollingRuntime::worker_loop, this);
    return true;
}

void ModbusPollingRuntime::stop() {
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

void ModbusPollingRuntime::submit_write(ControlCommand cmd) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pending_writes_.push_back(std::move(cmd));
    }
    cv_.notify_one();
}

void ModbusPollingRuntime::worker_loop()
{
    using clock = std::chrono::steady_clock;
    const auto poll_interval = std::chrono::milliseconds(poll_interval_ms_);

    auto next_poll_at = clock::now() + poll_interval;

    Logger::info("[ModbusPollingRuntime] worker started");

    while (running_)
    {
        ControlCommand write_cmd;
        bool has_write = false;

        {
            std::unique_lock<std::mutex> lock(mutex_);

            if (pending_writes_.empty())
            {
                cv_.wait_until(lock, next_poll_at, [this]() {
                    return !running_ || !pending_writes_.empty();
                });
            }

            if (!running_)
            {
                break;
            }

            if (!pending_writes_.empty())
            {
                write_cmd = pending_writes_.front();
                pending_writes_.pop_front();
                has_write = true;
            }
        }

        if (has_write)
        {
            if (!perform_write(write_cmd))
            {
                Logger::warn("[ModbusPollingRuntime] write failed, requeue for retry");

                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    pending_writes_.push_front(write_cmd);
                }

                wait_retry_delay(500);
                continue;
            }
        }

        const auto now = clock::now();
        if (now >= next_poll_at)
        {
            if (!perform_poll())
            {
                wait_retry_delay(500);
            }

            next_poll_at = clock::now() + poll_interval;
        }
    }

    mark_disconnected();
    Logger::info("[ModbusPollingRuntime] worker stopped");
}

bool ModbusPollingRuntime::ensure_connected() {
    if (connected_)
    {
        return true;
    }
    
    Logger::info("[ModbusPollingRuntime] connecting adapter");

    connected_ = adapter_->connect();
    if (!connected_)
    {
        Logger::error("[ModbusPollingRuntime] connect failed");
        return false;
    }
    return true;
}

void ModbusPollingRuntime::mark_disconnected() {
    if (!connected_)
    {
        return;
    }
    
    adapter_->disconnect();
    connected_ = false;
}

bool ModbusPollingRuntime::perform_poll() {
    if (!ensure_connected())
    {
        return false;
    }
    
    std::vector<uint16_t> regs;
    if (!adapter_->read_holding_registers(slave_id_, sample_base_addr_, 2, regs))
    {
        Logger::error("[ModbusPollingRuntime] read_holding_registers failed");
        mark_disconnected();
        return false;
    }
    
    if (regs.size() < 2)
    {
        Logger::error("[ModbusPollingRuntime] insufficient register data");
        return false;
    }
    
    DeviceSample sample;
    sample.device_id = "dev-" + std::to_string(slave_id_);
    sample.temperature = static_cast<float>(regs[0]) / 10.0f;
    sample.pressure = static_cast<float>(regs[1]) / 10.0f;

    Logger::info("[ModbusPollingRuntime] sample read: " + Logger::to_string(sample));

    bus_.publish(FrameworkEvents::MODBUS_SAMPLE_RECEIVED, "ModbusPollingRuntime", sample);

    return true;
}

bool ModbusPollingRuntime::perform_write(const ControlCommand& cmd) {
    if (!ensure_connected())
    {
        return false;
    }
    Logger::info("[ModbusPollingRuntime] write request: " + Logger::to_string(cmd));
    
    if (!adapter_->write_single_register(
        slave_id_,
        cmd.addr,
        static_cast<uint16_t>(cmd.value)))
    {
        Logger::error("[ModbusPollingRuntime] write_single_register failed");
        mark_disconnected();
        return false;
    }
    return true;
}

void ModbusPollingRuntime::wait_retry_delay(int delay_ms)
{
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait_for(lock, std::chrono::milliseconds(delay_ms), [this]() {
        return !running_;
    });
}
