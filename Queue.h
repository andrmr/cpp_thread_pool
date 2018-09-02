#pragma once

#include "Runnable.h"

#include <atomic>
#include <mutex>
#include <queue>

namespace TP {
namespace details {

auto static Comparer = [](IRunnable::Ptr left, IRunnable::Ptr right) { return left->priority > right->priority; };

class Queue
{
    std::priority_queue<IRunnable::Ptr, std::deque<IRunnable::Ptr>, decltype(details::Comparer)> m_tasks {details::Comparer};
    std::mutex m_mutex;
    std::atomic_bool m_continue {true};

    std::condition_variable m_condition;
    std::mutex m_conditionMutex;

public:
    auto enqueue(IRunnable::Ptr runnable)
    {
        {
            std::lock_guard<std::mutex> g(m_mutex);
            m_tasks.push(runnable);
        }

        m_condition.notify_one();
    }

    auto dequeue()
    {
        std::lock_guard<std::mutex> g(m_mutex);
        auto task = m_tasks.top();
        m_tasks.pop();

        return task;
    }

    auto empty()
    {
        std::lock_guard<std::mutex> g(m_mutex);
        return m_tasks.empty();
    }

    auto stop()
    {
        m_continue = false;
        m_condition.notify_all();
    }

    auto run()
    {
        while (true)
        {
            std::unique_lock<std::mutex> g(m_conditionMutex);
            m_condition.wait(g, [this]{ return !empty(); }); /// @todo this condition check hangs; fix this bug
            g.unlock();

            if (m_continue)
            {
                auto task = dequeue();
                task->run();
            }
        }
    }
};

}} // TP::details
