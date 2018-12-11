cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(Data-Comm)

set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

add_subdirectory(Assignments)
add_subdirectory(benchmark)
