cmake_minimum_required(VERSION 3.14)
project(undis VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(undis)

enable_testing()
add_subdirectory(undis_test)
