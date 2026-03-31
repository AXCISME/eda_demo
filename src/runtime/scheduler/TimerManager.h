#pragma once
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include "runtime/logging/Logger.h"

struct TimerTask
{
    std::string name;
    int interval_ms {1000};
    std::chrono::steady_clock::time_point next_run;
    std::function<void()> callback;
};

class TimerManager
{
public:
    void add_periodic_timer(const std::string& name, int interval_ms, std::function<void()> cb) {
        TimerTask task;
        task.name = name;
        task.interval_ms = interval_ms;
        task.next_run = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval_ms);
        task.callback = std::move(cb);

        timers_.push_back(std::move(task));
    }

    void process() {
        auto now = std::chrono::steady_clock::now();

        for (auto& timer : timers_)
        {
            if (now >= timer.next_run)
            {
                Logger::info("[TimerManager] timer fired: " + timer.name);
                timer.callback();
                timer.next_run = now + std::chrono::milliseconds(timer.interval_ms);
            }
        }
    }

private:
    std::vector<TimerTask> timers_;
};
