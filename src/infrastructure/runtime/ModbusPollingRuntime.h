#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>

#include "runtime/bus/EventBus.h"
#include "infrastructure/transport/modbus/IModbusMasterAdapter.h"

class ModbusPollingRuntime {
public:
    ModbusPollingRuntime(EventBus& bus,
                         std::unique_ptr<IModbusMasterAdapter> adapter,
                         int poll_interval_ms,
                         int slave_id = 1,
                         int sample_base_addr = 100);

    ~ModbusPollingRuntime();

    bool start();
    void stop();

    // 线程安全：bus 线程、HTTP 线程、WS 线程都可以调用
    void submit_write(ControlCommand cmd);

private:
    void worker_loop(); // 主循环

    bool ensure_connected();    // 确保通信连接可用
    void mark_disconnected();   // 标记为连接断开

    bool perform_poll();    // 执行一次轮询操作
    bool perform_write(const ControlCommand& cmd);  // 执行一次写操作

    void wait_retry_delay(int delay_ms);    // 失败后延迟等待

private:
    EventBus& bus_;
    std::unique_ptr<IModbusMasterAdapter> adapter_;

    int poll_interval_ms_ {1000};
    int slave_id_ {1};
    int sample_base_addr_ {100};

    std::atomic<bool> running_ {false};
    bool connected_ {false};

    std::thread worker_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::deque<ControlCommand> pending_writes_;
};
