#include "ThreadPool.h"

#include <iostream>
#include <sstream>
#include <string>

std::string func(int a, int b, int c)
{
    std::stringstream out;
    out << __FUNCTION__ << " on thread " << std::this_thread::get_id() << " \tResult: " << a + b + c;
    return out.str();
}

int main()
{
    TP::ThreadPool tp(std::thread::hardware_concurrency());

    std::vector<std::future<std::string>> futures;
    for (size_t i = 0; i < 100; ++i)
    {
        futures.push_back(tp.addTask(func, i, i + 1, i + 2));
    }

    for (auto& f: futures)
    {
        std::cout << f.get() << '\n';
    }

    tp.stop();
    return 0;
}
