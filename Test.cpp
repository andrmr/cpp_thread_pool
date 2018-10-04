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
    TP::ThreadPool tp;

    auto f1 = tp.addTask(func1<uint32_t>, 9u);

    uint16_t y = 5;
    auto f2 = tp.addTask(func2, 10, y, 5);

    f1.get();
    f2.get();

    return 0;
}
