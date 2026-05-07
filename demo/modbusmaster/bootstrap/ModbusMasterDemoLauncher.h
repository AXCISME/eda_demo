#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <vector>

#include "application/services/ControlService.h"
#include "bootstrap/ApplicationBootstrap.h"
#include "demo/modbusmaster/polling/ModbusPollingModule.h"
#include "domain/events/DomainEvent.h"
#include "runtime/logging/Logger.h"
#include "runtime/scheduler/TimerManager.h"

struct SerialPortConfig {
    std::string device_path;
    int baud_rate;
    char parity;
    int data_bits;
    int stop_bits;
    int gpio_pin;

    SerialPortConfig() : device_path("/dev/ttyS4"), baud_rate(9600),
                         parity('N'), data_bits(8), stop_bits(1), gpio_pin(111) {}
};

inline bool configure485GPIO(int gpio_pin)
{
    std::string gpio_path = "/sys/class/gpio/gpio" + std::to_string(gpio_pin);

    struct stat st;
    if (stat(gpio_path.c_str(), &st) != 0)
    {
        std::string export_cmd = "echo " + std::to_string(gpio_pin) +
                                 " | tee /sys/class/gpio/export";
        if (system(export_cmd.c_str()) != 0)
        {
            return false;
        }
    }

    std::string direction_cmd = "echo out | tee " + gpio_path + "/direction";
    if (system(direction_cmd.c_str()) != 0)
    {
        return false;
    }

    std::string value_cmd = "echo 1 | tee " + gpio_path + "/value";
    if (system(value_cmd.c_str()) != 0)
    {
        return false;
    }

    return true;
}

inline int run_modbusmaster_demo()
{
    SerialPortConfig serial_cfg;

    if (!configure485GPIO(serial_cfg.gpio_pin))
    {
        Logger::error("[ModbusMasterDemo] GPIO 485 config failed");
        return 1;
    }

    AppConfig config;
    config.http.enabled = false;
    config.http.backend = HttpBackendId::FAKE;
    config.http.host = "0.0.0.0";
    config.http.port = 8080;
    config.modbus_master.enabled = true;
    config.modbus_master.devices.push_back(ModbusDeviceConfig {
        "demo-device",
        ModbusMasterBackendId::LIBMODBUS_MASTER_RTU,
        "",                                         // tcp_host（不使用）
        0,                                          // tcp_port（不使用）
        serial_cfg.device_path,                     // /dev/ttyS4
        serial_cfg.baud_rate,                       // 9600
        serial_cfg.parity,                          // N
        serial_cfg.data_bits,                       // 8
        serial_cfg.stop_bits                        // 1
    });

    auto host = ApplicationBootstrap::create(
        config,
        {},
        [](EventBus& bus) -> std::unique_ptr<TimerManager> {
            auto timers = std::make_unique<TimerManager>();

            timers->add_periodic_timer("modbus_poll", 1000, [&bus]() {
                ModbusReadRequest req;
                req.request_id = "poll." + std::to_string(
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()
                    ).count());
                req.device_name = "demo-device";                        // 设备名称
                req.reg_type = ModbusRegisterType::HOLDING_REGISTER;    // 寄存器类型
                req.slave_id = 1;                                       // 从机地址
                req.addr = 100;                                         // 寄存器地址
                req.count = 2;                                          // 读取数量

                bus.publish(FrameworkEvents::READ_REQUEST, "ModbusDemoTimer", std::move(req));
            });

            return timers;
        },
        [](EventBus& bus) -> std::vector<std::unique_ptr<IBusinessModule>> {
            std::vector<std::unique_ptr<IBusinessModule>> modules;
            modules.push_back(std::make_unique<ControlService>(bus));
            modules.push_back(std::make_unique<ModbusPollingModule>(bus, "demo-device"));
            return modules;
        });
    if (!host)
    {
        Logger::error("[ModbusMasterDemo] application bootstrap failed");
        return 1;
    }
    host->init();

    Logger::info("[ModbusMasterDemo] running for 60 seconds");

    std::thread stopper([app = host.get()]() {
        std::this_thread::sleep_for(std::chrono::seconds(60));
        app->stop();
    });

    host->run();

    if (stopper.joinable())
    {
        stopper.join();
    }

    return 0;
}
