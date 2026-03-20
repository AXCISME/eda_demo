#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include "core/Event.h"

class EventQueue {
public:
    void push(const Event& event) {     // 生产者调用
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(event);
        }   // 这里多包了一层，是为了让lock提前析构
        cv_.notify_one();   // 通知一个正在等待的线程
    }

    bool wait_and_pop(Event& out, int timeout_ms) {     // 消费者调用
        std::unique_lock<std::mutex> lock(mutex_);

        if (!cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this](){
            return !queue_.empty();
        }))
        {
            return false;
        }
        
        out = queue_.front();
        queue_.pop();
        return true;
    }

private:
    std::queue<Event> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};
