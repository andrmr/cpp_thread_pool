#pragma once

#include "Priority.h"

#include <functional>
#include <future>
#include <memory>

namespace TP {
namespace details {

struct IRunnable
{
    using Ptr = std::shared_ptr<IRunnable>;

    virtual void run() = 0;
    virtual ~IRunnable() = default;

    IRunnable(Priority priority = Priority::LOW)
        : priority {priority} {}

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
    std::packaged_task<result_t()> m_task; // workaround for std::bind; instead of result_t(decay_t<Args>...)

public:
    using Ptr = std::shared_ptr<Runnable<Callable, Args...>>;

    Runnable(Priority priority, Callable&& c, Args&&... args)
        : IRunnable(priority),
          m_task{std::bind(std::forward<Callable>(c), std::forward<Args>(args)...)}
    {
    }

    Runnable(Callable&& c, Args&&... args)
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

}} // TP::details
