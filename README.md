# cpp_thread_pool
Thread pool written in C++ 17

Runs queued tasks on a given number of reusable threads.

#### Usage:
```cpp
#include "ThreadPool.h"

#include <iostream>

auto free_func(int a, int b, int c) {
    return a + b + c;
}

auto lambda_func = [](auto&&... args) -> typename std::common_type<decltype(args)...>::type {
    return (args + ...);
};

struct Pod {
    auto mem_func(int a, int b, int c)
    {
        return a + b + c;
    }
};

int main() {
    TP::ThreadPool tp(std::thread::hardware_concurrency());

    // queue free function
    auto f = tp.addTask(free_func, 1, 2, 3);

    // queue variadic lambda or template wrapper
    auto g = tp.addTask(lambda_func, 1, 2, 3.5);

    // queue member function
    auto h = tp.addTask(std::mem_fn(&Pod::mem_func), Pod {}, 1, 2, 3);

    std::cout << f.get() << '\n' << g.get() << '\n' << h.get() << '\n';

    tp.stop();
    return 0;
}
```

#### TODO:
 - create more tests or move to a UT framework
 - improve flexibility i.e. overloads, dynamic resizing
