cmake_minimum_required(VERSION 3.16)

project(safe-mutex CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_compile_options(-Wall -Wextra -Werror)
add_compile_options(-fsanitize=address -fsanitize=undefined)
add_link_options(-fsanitize=address -fsanitize=undefined)

find_package(Threads)
include_directories(include)

add_executable(safe-mutex-tests src/safe_mutex.cpp 
                                src/safe_mutex_tests.cpp 
                                src/doctest_main.cpp)
target_compile_options(safe-mutex-tests PUBLIC -DSAFE_MUTEX_TESTS_)
target_link_libraries(safe-mutex-tests ${CMAKE_THREAD_LIBS_INIT})

add_executable(deadlock-main src/safe_mutex.cpp 
                             src/deadlock_main.cpp)
target_link_libraries(deadlock-main ${CMAKE_THREAD_LIBS_INIT})
