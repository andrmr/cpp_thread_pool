# cpp_thread_pool
Thread pool written in C++ 14

Runs queued tasks on a given number of reusable threads.

#### Usage:
```cpp

template <typename T, typename U>
int func1(T arg1, U arg2);

template <typename T, typename U, typename V>
double func2(T arg1, U arg2, V arg3);

TP::ThreadPool tp (std::thread::hardware_concurrency());

std::vector<std::future<int>> integers;
std::vector<std::future<double>> doubles;

// queue any non-void functions
for (size_t i = 0; i < 1000; ++i) {
    integers.push_back(tp.addTask(func1, x, y));
    doubles.push_back(tp.addTask(func2, x, y, z));
}

// do stuff while tasks are running

// get the results from the returned std::futures
for (auto i: integers) {
    result.get();
}

tp.stop();
```

#### TODO:
 - create more tests or move to a UT framework
 - improve flexibility i.e. overloads, dynamic resizing, move away from bind
 - cleanup and comments
 - migrate to C++ 17 features
