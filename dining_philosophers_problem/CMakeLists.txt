cmake_minimum_required(VERSION 3.1)
enable_language(C)
project(DiningPhilosophersProblem)

set(CMAKE_C_FLAGS "-g3 -Wall")

add_library(lib_dpp_thread STATIC dpp.c)
target_compile_definitions(lib_dpp_thread PRIVATE MODE_THREAD)
target_link_libraries(lib_dpp_thread PRIVATE pthread)
add_executable(dpp_thread thread.c)
target_link_libraries(dpp_thread PRIVATE  lib_dpp_thread)

add_library(lib_dpp_process STATIC dpp.c)
target_compile_definitions(lib_dpp_process PRIVATE MODE_PROCESS)
target_link_libraries(lib_dpp_process PRIVATE pthread)
add_executable(dpp_process process.c)
target_link_libraries(dpp_process PRIVATE lib_dpp_process)

