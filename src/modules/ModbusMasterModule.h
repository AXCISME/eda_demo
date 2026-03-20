#pragma once
#include <condition_variable>
#include <chrono>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>
#include "core/EventBus.h"
#include "core/Logger.h"
#include "transport/modbus/IModbusMasterAdapter.h"

class ModbusMasterModule
{
public:
    ModbusMasterModule(EventBus& bus, std::unique_ptr<IModbusMasterAdapter> adapter)
        : bus_(bus),
          adapter_(std::move(adapter))
    {
    }

    void init()
    {
        bus_.subscribe(EventType::CONTROL_COMMAND, [this](const Event& e) {
            on_control_command(e);
        });
    }

    bool start()
    {
        if (!adapter_)
        {
            Logger::error("[ModbusMasterModule] adapter is null");
            return false;
        }

        if (!adapter_->connect())
        {
            return false;
        }

        bool expected = false;
        if (!running_.compare_exchange_strong(expected, true))
        {
            return true;
        }

        worker_ = std::thread([this]() {
            worker_loop();
        });
        return true;
    }

    void stop()
    {
        bool expected = true;
        if (running_.compare_exchange_strong(expected, false))
        {
            cv_.notify_all();
            if (worker_.joinable())
            {
                worker_.join();
            }
        }

        if (adapter_)
        {
            adapter_->disconnect();
        }
    }

private:
    void worker_loop()
    {
        while (running_)
        {
            process_pending_writes();
            poll_once();

            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait_for(lock, std::chrono::milliseconds(poll_interval_ms_), [this]() {
                return !running_ || !pending_writes_.empty();
            });
        }
    }

    void poll_once()
    {
        if (!adapter_)
        {
            Logger::error("[ModbusMasterModule] adapter is null");
            return;
        }

        std::vector<uint16_t> regs;
        if (!adapter_->read_holding_registers(default_slave_id_, sample_base_addr_, 2, regs))
        {
            Logger::error("[ModbusMasterModule] failed to read holding registers");
            return;
        }

        if (regs.size() < 2)
        {
            Logger::error("[ModbusMasterModule] insufficient register data");
            return;
        }

        DeviceSample sample;
        sample.device_id = "dev-" + std::to_string(default_slave_id_);
        sample.temperature = static_cast<float>(regs[0]) / 10.0f;
        sample.pressure = static_cast<float>(regs[1]) / 10.0f;

        Logger::info("[ModbusMasterModule] sample read: " + Logger::to_string(sample));

        bus_.publish(Event(
            EventType::MODBUS_DATA_RECEIVED,
            "ModbusMasterModule",
            sample
        ));
    }

    void on_control_command(const Event& e)
    {
        if (auto cmd = std::get_if<ControlCommand>(&e.data))
        {
            Logger::info("[ModbusMasterModule] received control command: " + Logger::to_string(*cmd));
            {
                std::lock_guard<std::mutex> lock(mutex_);
                pending_writes_.push_back(*cmd);
            }
            cv_.notify_one();
        }
        else
        {
            Logger::warn("[ModbusMasterModule] CONTROL_COMMAND data type mismatch");
        }
    }

    void process_pending_writes()
    {
        if (!adapter_)
        {
            Logger::error("[ModbusMasterModule] adapter is null");
            return;
        }

        std::deque<ControlCommand> writes;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            writes.swap(pending_writes_);
        }

        for (const auto& cmd : writes)
        {
            if (!adapter_->write_single_register(
                    default_slave_id_,
                    cmd.addr,
                    static_cast<uint16_t>(cmd.value)))
            {
                Logger::error("[ModbusMasterModule] write single register failed");
            }
        }
    }

private:
    EventBus& bus_;
    std::unique_ptr<IModbusMasterAdapter> adapter_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::deque<ControlCommand> pending_writes_;
    std::atomic<bool> running_ {false};
    std::thread worker_;
    int default_slave_id_ {1};
    int sample_base_addr_ {100};
    int poll_interval_ms_ {1000};
};
