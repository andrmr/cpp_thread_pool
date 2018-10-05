#include "ThreadPool.h"
#include <iostream>

template <typename T>
T func1(T x)
{
    std::cout << __PRETTY_FUNCTION__ << " on thread: " << std::this_thread::get_id() << "\n";
    return x;
}

uint32_t func2(uint32_t x, uint16_t y, uint8_t z)
{
    std::cout << __PRETTY_FUNCTION__ << " on thread: " << std::this_thread::get_id() << "\n";
    return (x + y + z);
}

int main()
{
    TP::ThreadPool tp (std::thread::hardware_concurrency());

    std::vector<std::future<uint32_t>> futures;

    for (size_t i = 0; i < 100; ++i)
    {
        futures.push_back(tp.addTask(func2, i, i+1, i+2));
    }

    for (auto&& f: futures)
    {
        f.get();
    }

    tp.stop();
    return 0;
}
