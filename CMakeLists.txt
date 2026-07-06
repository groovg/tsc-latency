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

if(PROJECT_IS_TOP_LEVEL)
    enable_testing()

    foreach(t tsc_test clock_test bench_test histogram_test)
        add_executable(${t} test/${t}.cpp)
        target_link_libraries(${t} PRIVATE tsclat)
        target_compile_options(${t} PRIVATE -Wall -Wextra)
        add_test(NAME ${t} COMMAND ${t})
    endforeach()

    foreach(ex measure_overhead measure_function)
        add_executable(${ex} examples/${ex}.cpp)
        target_link_libraries(${ex} PRIVATE tsclat)
    endforeach()
endif()
