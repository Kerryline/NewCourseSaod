cmake_minimum_required(VERSION 3.16)
project(NewCourseSaod LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(saod
    src/main.cpp
    src/heap_sort.cpp
    src/encoding.cpp
    src/database.cpp
    src/view.cpp
)

target_include_directories(saod PRIVATE include)
target_compile_options(saod PRIVATE -Wall -Wextra)

option(BUILD_TESTS "Build algorithm tests" OFF)

if(BUILD_TESTS)
    enable_testing()
    add_executable(heap_sort_tests
        tests/test_heap_sort.cpp
        src/heap_sort.cpp
    )
    target_include_directories(heap_sort_tests PRIVATE include)
    target_compile_options(heap_sort_tests PRIVATE -Wall -Wextra)
    add_test(NAME heap_sort COMMAND heap_sort_tests)
endif()
