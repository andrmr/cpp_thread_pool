cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_STANDARD 17)

project(cpp_thread_pool)
add_executable(${PROJECT_NAME} "Test.cpp" "ThreadPool.h")

find_package(Threads)
target_link_libraries(${PROJECT_NAME} Threads::Threads)
