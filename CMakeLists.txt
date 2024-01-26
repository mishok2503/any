cmake_minimum_required(VERSION 3.27)
project(any)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_MODULE_PATH  "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Catch REQUIRED)

add_executable(test test.cpp any.cpp)

target_link_libraries(test PRIVATE Catch2::Catch2WithMain)