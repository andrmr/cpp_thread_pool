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

namespace impl {

struct IRunnable
{
    using Ptr = std::shared_ptr<IRunnable>;

    virtual void run()   = 0;
    virtual ~IRunnable() = default;

    explicit IRunnable(Priority priority = Priority::DEFAULT)
        : priority {priority}
    {
    }

    Priority const priority;
};

template <typename T>
using decay_t = typename std::decay<T>::type;

template <typename Callable, typename... Args>
class Runnable: public IRunnable
{
public:
	using result_t = typename std::invoke_result<decay_t<Callable>, decay_t<Args>...>::type;
    
    explicit Runnable(Priority priority, Callable&& c, Args&&... args)
		: IRunnable {priority},
		m_task {[&c, &args...] { return c(std::forward<Args>(args)...); }} // captures by &&
    {
    }

    explicit Runnable(Callable&& c, Args&&... args)
        : Runnable(Priority::DEFAULT, std::forward<Callable>(c), std::forward<Args>(args)...)
    {
    }

    auto getFuture() -> std::future<result_t>
    {
        return m_task.get_future();
    }

    void run() override
    {
        m_task();
    }

private:
	std::packaged_task<result_t()> m_task;
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
    auto enqueue(Priority priority, Callable&& c, Args&&... args) -> std::future<typename Runnable<Callable, Args...>::result_t>
    {
        auto runnable = std::make_shared<Runnable<Callable, Args...>>(priority, std::forward<Callable>(c), std::forward<Args>(args)...);

        {
			std::lock_guard g {m_mutex};
            m_tasks.push(runnable);
        }

        m_condition.notify_one();
        return runnable->getFuture();
    }

    auto empty() noexcept -> bool
    {
		std::lock_guard g {m_mutex};
        return m_tasks.empty();
    }

    auto stop() noexcept
    {
        m_continue = false;
        m_condition.notify_all();
    }

    auto run() noexcept
    {
        while (m_continue)
        {
            std::unique_lock lk {m_condition_mutex};
            m_condition.wait(lk, [this] { return !empty() || !m_continue; });
            lk.unlock();

            std::unique_lock g {m_mutex};
            if (!m_tasks.empty())
            {
                auto task = m_tasks.top();
                m_tasks.pop();
                g.unlock();

                m_condition.notify_one();
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
    impl::Queue m_queue;
    std::vector<std::thread> m_threads;

public:
    explicit ThreadPool(unsigned int maxThreads = std::thread::hardware_concurrency())
		: m_threads {maxThreads}
    {
        for (auto& t: m_threads)
        {
            t = std::move(std::thread(&impl::Queue::run, &m_queue));
        }
    }

    template <typename Callable, typename... Args, typename = std::enable_if<std::is_invocable<Callable, Args...>::value>::type>
    auto addTask(Priority priority, Callable&& c, Args&&... args) -> std::future<typename impl::Runnable<Callable, Args...>::result_t>
    {
        return m_queue.enqueue(priority, std::forward<Callable>(c), std::forward<Args>(args)...);
    }

    template <typename Callable, typename... Args>
    auto addTask(Callable&& c, Args&&... args)
    {
        return addTask(Priority::DEFAULT, std::forward<Callable>(c), std::forward<Args>(args)...);
    }

    auto stop() noexcept
    {
        m_queue.stop();
        for (auto& t: m_threads)
        {
			if (t.joinable())
			{
				t.join();
			}
        }
    }
};

} // namespace TP
