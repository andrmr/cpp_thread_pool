# cpp_thread_pool
Thread pool written in C++ 14

Runs queued tasks on a given number of reusable threads.

#### Usage:
```cpp
std::string func(int a, int b, int c);

TP::ThreadPool tp;

std::vector<std::future<std::string>> futures;
for (size_t i = 0; i < 100; ++i) {
    futures.push_back(tp.addTask(func, i, i + 1, i + 2));
}

for (auto& f: futures) {
    std::cout << f.get() << '\n';
}

tp.stop();
```

#### TODO:
 - allow templated functions to be queued
 - create more tests or move to a UT framework
 - improve flexibility i.e. overloads, dynamic resizing, move away from bind
 - cleanup and comments
 - migrate to C++ 17 features
