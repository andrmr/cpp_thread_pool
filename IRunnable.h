#pragma once

enum class Priority
{
    URGENT = 0,
    HIGH   = 1,
    MEDIUM = 2,
    LOW    = 3
};

struct IRunnable
{
    virtual void run() = 0;
    virtual ~IRunnable() = default;

    IRunnable(Priority priority = Priority::LOW)
        : priority {priority} {}

    Priority priority;
};
