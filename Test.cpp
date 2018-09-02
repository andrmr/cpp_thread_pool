#include "ThreadPool.h"

#include <iostream>

using namespace TP;
using namespace TP::details;

int func(int b)
{
    std::cout << "From func";
    return b;
}

//template <typename Callable, typename... Args>
//void runTask(Priority priority, Callable&& c, Args&&... args)
//{
//    auto runnable = Runnable<Callable, Args...>(priority, std::forward<Callable>(c), std::forward<Args>(args)...);
//    runnable.run();
//}

int main()
{
    int b = 1;

    TP::ThreadPool tp;
    auto f = tp.addTask(func, b);
    f.get();

    return 0;
}
