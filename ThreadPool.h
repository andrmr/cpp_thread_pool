#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <utility>
#include <vector>

namespace TP {

enum class Priority
{
    URGENT = 0,
    HIGH,
    MEDIUM,
    LOW,

    DEFAULT = LOW
};

namespace details {

struct IRunnable
{
    using Ptr = std::shared_ptr<IRunnable>;

    virtual void run()   = 0;
    virtual ~IRunnable() = default;

    explicit IRunnable(Priority priority = Priority::DEFAULT)
        : priority{priority} {}

    Priority priority;
};

template <typename T>
using decay_t = typename std::decay<T>::type;

template <typename T>
using result_of_t = typename std::result_of<T>::type;

template <typename Callable, typename... Args>
class Runnable : public IRunnable
{
    using result_t = result_of_t<decay_t<Callable>(decay_t<Args>...)>;
    std::packaged_task<result_t()> m_task; // result of std::bind; otherwise result_t(decay_t<Args>...)

public:
    using Ptr = std::shared_ptr<Runnable<Callable, Args...>>;

    Runnable(Priority priority, Callable&& c, Args&&... args)
        : IRunnable(priority),
          m_task{std::bind(std::forward<Callable>(c), std::forward<Args>(args)...)}
    {
    }

    explicit Runnable(Callable&& c, Args&&... args)
        : m_task{std::bind(std::forward<Callable>(c), std::forward<Args>(args)...)}
    {
    }

    auto getFuture()
    {
        return m_task.get_future();
    }

    void run() override
    {
        m_task();
    }
};

class Queue
{
    std::function<bool(IRunnable::Ptr, IRunnable::Ptr)> comparer =
        [](IRunnable::Ptr left, IRunnable::Ptr right) { return left->priority < right->priority; };

    std::priority_queue<IRunnable::Ptr, std::deque<IRunnable::Ptr>, decltype(comparer)> m_tasks {comparer};
    std::mutex m_mutex;
    std::mutex m_condition_mutex;
    std::condition_variable m_condition;
    std::atomic_bool m_continue {true};

public:
    template <typename Callable, typename... Args>
    auto enqueue(Priority priority, Callable&& c, Args&&... args)
    {
        auto runnable = std::make_shared<Runnable<Callable, Args...>>(priority, std::forward<Callable>(c), std::forward<Args>(args)...);
        auto future   = runnable->getFuture();

        {
            std::lock_guard<std::mutex> g(m_mutex);
            m_tasks.push(runnable);
        }

        m_condition.notify_one();
        return future;
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
        while (m_continue)
        {
            std::unique_lock<std::mutex> g(m_condition_mutex);
            m_condition.wait(g, [this] { return !empty() || !m_continue; });
            g.unlock();

            if (!empty())
            {
                auto task = dequeue();
                task->run();
            }
        }
    }
};

} // namespace details

/// Manages a given number of threads and runs a task queue.
/// The number of threads is limited to maximum hardware threads.
class ThreadPool
{
    details::Queue m_queue;
    std::vector<std::thread> m_threads;

public:
    explicit ThreadPool(unsigned int maxThreads = 1)
        : m_threads(std::min(maxThreads, std::thread::hardware_concurrency()))
    {
        for (auto& t : m_threads)
        {
            t = std::move(std::thread(&details::Queue::run, &m_queue));
        }
    }

    template <typename Callable, typename... Args>
    auto addTask(Priority priority, Callable&& c, Args&&... args)
    {
        return m_queue.enqueue(priority, std::forward<Callable>(c), std::forward<Args>(args)...);
    }

    template <typename Callable, typename... Args>
    auto addTask(Callable&& c, Args&&... args)
    {
        return addTask(Priority::DEFAULT, std::forward<Callable>(c), std::forward<Args>(args)...);
    }

    auto stop()
    {
        m_queue.stop();
        for (auto& t : m_threads)
        {
            if (t.joinable()) t.join();
        }
    }
};

}  // namespace TP
