#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H

#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <deque>
#include <optional>
#include "Message.h"

// we just avoid copying the threadsafequeue from Javidx9 so we implement this with
// the following doc (+os lessons bases)
// https://stackoverflow.com/questions/51372861/is-stdmutex-as-a-member-variable-thread-safe-for-multiple-threads
// https://en.cppreference.com/w/cpp/thread/lock_guard
// Using a template for a better flexibilty !
template <typename type>
class ThreadSafeQueue
{
public:
    size_t size()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    bool is_empty() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    // warning we pushing const i don't know if we want
    // to modify message later.
    void push(const type &msg)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(msg);
    }

    // https://codetrips.com/2020/07/26/modern-c-writing-a-thread-safe-queue/
    // please ensure to use if (msg.has_value()) to ensure that your are not
    // manipulating a not initialised empty msg !
    std::optional<type> pop()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty())
        {
            return {};
        }
        type instance = queue_.front();
        queue_.pop_front();
        return instance;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.clear();
    }

private:
    std::mutex mutex_;
    std::deque<type> queue_;
};

#endif // THREADSAFEQUEUE_H