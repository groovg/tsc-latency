cmake_minimum_required(VERSION 3.20)
project(tsc_latency CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Header-only library.
add_library(tsclat INTERFACE)
target_include_directories(tsclat INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(tsclat INTERFACE cxx_std_20)

enable_testing()
