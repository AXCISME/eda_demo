#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <variant>
#include <vector>

#include "bootstrap/AppConfig.h"
#include "domain/model/EventData.h"
#include "infrastructure/transport/modbus/IModbusMasterAdapter.h"
#include "runtime/bus/EventBus.h"

class ModbusDeviceRuntime
{
public:
    ModbusDeviceRuntime(
        EventBus& bus,
        std::unique_ptr<IModbusMasterAdapter> adapter,
        const ModbusDeviceConfig& config);

    ~ModbusDeviceRuntime();

    bool start();
    void stop();

    void submit_read(ModbusReadRequest req);
    void submit_write(ModbusWriteRequest req);

    const std::string& device_name() const;

private:
    using PendingOperation = std::variant<ModbusReadRequest, ModbusWriteRequest>;

    void worker_loop();

    bool ensure_connected();
    void mark_disconnected();
    void wait_retry_delay();

    bool perform_read(const ModbusReadRequest& req);
    bool perform_write(const ModbusWriteRequest& req);
    void publish_failure(std::string request_id, std::string error_message);

    bool do_read(
        ModbusRegisterType type,
        int slave,
        int addr,
        int count,
        std::vector<uint16_t>& out);

    bool do_write(const ModbusWriteRequest& req);

private:
    EventBus& bus_;
    std::unique_ptr<IModbusMasterAdapter> adapter_;
    ModbusDeviceConfig config_;

    std::atomic<bool> running_ {false};
    bool connected_ {false};

    std::thread worker_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::deque<PendingOperation> pending_ops_;
};
