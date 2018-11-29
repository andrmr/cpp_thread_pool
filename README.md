# cpp_thread_pool
Thread pool written in C++ 17

Runs queued tasks on a given number of reusable threads.

#### Usage:
```cpp
TP::ThreadPool tp;

// regular function
std::string func(int a, int b, int c);

std::vector<std::future<std::string>> futures;
for (int i = 0; i < 100; ++i) {
    futures.push_back(tp.addTask(func, i, i + 1, i + 2));
}

for (auto& f: futures) {
    std::cout << f.get() << '\n';
}

// function templates must be wrapped in lambdas
template <typename... Args>
constexpr auto sum(Args&&... args) -> typename std::common_type<Args...>::type {
	return (args + ...);
}

auto wrapSum = [](auto&&... args) {
	return sum(std::forward<decltype(args)>(args)...);
};

auto f = tp.addTask(sum, 1, 2, 3.5f);
std::cout << f.get();

tp.stop();
```

#### TODO:
 - create more tests or move to a UT framework
 - improve flexibility i.e. overloads, dynamic resizing
