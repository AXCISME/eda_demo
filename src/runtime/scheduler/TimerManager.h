#pragma once
#include <chrono>
#include <ctime>
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

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
                Logger::info("[TimerManager] timer fired: " + timer.name + " at " + format_now());
                timer.callback();
                timer.next_run = now + std::chrono::milliseconds(timer.interval_ms);
            }
        }
    }

private:
    static std::string format_now() {
        const auto now = std::chrono::system_clock::now();
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        const std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &now_time);
#else
        localtime_r(&now_time, &tm);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
            << '.'
            << std::setw(3)
            << std::setfill('0')
            << ms.count();
        return oss.str();
    }

    std::vector<TimerTask> timers_;
};
