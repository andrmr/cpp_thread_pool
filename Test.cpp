#include "ThreadPool.h"

#include <iostream>
#include <sstream>
#include <string>
#include <thread>

std::string func(int a, int b, int c)
{
    std::stringstream out;
    out << __FUNCTION__ << " on thread " << std::this_thread::get_id() << " \tResult: " << a + b + c;
    return out.str();
}

template <typename... Args>
constexpr auto sum(Args&&... args) -> typename std::common_type<Args...>::type
{
	return (args + ...);
}

int main()
{
    TP::ThreadPool tp(std::thread::hardware_concurrency());

    std::vector<std::future<std::string>> futures;
    for (int i = 0; i < 100; ++i)
    {
        futures.push_back(tp.addTask(func, i, i + 1, i + 2));
    }

	for (auto& f : futures)
	{
		std::cout << f.get() << '\n';
	}

	auto simpleSumL = [](auto&&... args) {
		std::cout << "From lambda: " << sum(std::forward<decltype(args)>(args)...) << '\n';
	};

	auto f = tp.addTask(simpleSumL, 1, 2, 3.5);
	f.get();

	tp.stop();

	system("pause");
    return 0;
}
