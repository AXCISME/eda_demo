#include "demo/timer_test/TimerDemoLauncher.h"
#include "demo/timer_test/TimerDemoEvents.h"
#include "demo/timer_test/heartbest/HeartbestModule.h"
#include "demo/timer_test/update/UpdateModule.h"

#include <vector>

int run_timer_demo()
{
    AppConfig config;
    config.http.enabled = false;
    config.modbus_master.enabled = false;

    auto host = ApplicationBootstrap::create(
        config,
        {},
        [](EventBus& bus) -> std::unique_ptr<TimerManager> {
            auto timers = std::make_unique<TimerManager>();

            timers->add_periodic_timer("tick_1s", 1000, [&bus]() {
                bus.publish(
                    TimerDemoEvents::TICK_1S,
                    "TimerDemoTimer",
                    TimerDemoEvents::TimerTickPayload {"tick_1s", 1000});
            });

            timers->add_periodic_timer("tick_5s", 5000, [&bus]() {
                bus.publish(
                    TimerDemoEvents::TICK_5S,
                    "TimerDemoTimer",
                    TimerDemoEvents::TimerTickPayload {"tick_5s", 5000});
            });

            timers->add_periodic_timer("tick_10s", 10000, [&bus]() {
                bus.publish(
                    TimerDemoEvents::TICK_10S,
                    "TimerDemoTimer",
                    TimerDemoEvents::TimerTickPayload {"tick_10s", 10000});
            });

            return timers;
        },
        [](EventBus& bus) -> std::vector<std::unique_ptr<IBusinessModule>> {
            std::vector<std::unique_ptr<IBusinessModule>> modules;
            modules.push_back(std::make_unique<HeartbestModule>(bus));
            modules.push_back(std::make_unique<UpdateModule>(bus));
            return modules;
        }
    );
    if (!host)
    {
        Logger::error("[TimerDemo] application bootstrap failed");
        return 1;
    }

    host->init();
    Logger::info("[TimerDemo] running continuously; stop the process to exit");
    host->run();

    return 0;
}
