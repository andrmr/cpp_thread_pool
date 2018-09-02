#pragma once

#include "Queue.h"

#include <memory>
#include <utility>
#include <vector>

namespace TP {

/// Manages a given number of threads and runs a task queue.
class ThreadPool
{
    details::Queue m_queue;
    std::vector<std::thread> m_threads;

public:
    ThreadPool(size_t maxThreads = 1)
        : m_threads(maxThreads)
    {
        for (auto& t: m_threads)
        {
            t = std::move(std::thread(&details::Queue::run, &m_queue));
        }
    }

    template <typename Callable, typename... Args>
    auto addTask(Priority priority, Callable&& c, Args&&... args)
    {
        auto runnable = std::make_shared<details::Runnable<Callable, Args...>>(priority, std::forward<Callable>(c), std::forward<Args>(args)...);
        m_queue.enqueue(runnable);
        return runnable->getFuture();
    }

    template <typename Callable, typename... Args>
    auto addTask(Callable&& c, Args&&... args)
    {
        return addTask(Priority::LOW, std::forward<Callable>(c), std::forward<Args>(args)...);
    }

    auto stop()
    {
        m_queue.stop();
        for (auto& t: m_threads)
        {
            t.join();
        }
    }

    ~ThreadPool()
    {
        stop();
    }
};

}
