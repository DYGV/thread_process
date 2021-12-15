cmake_minimum_required(VERSION 3.1)
enable_language(C)
project(ProducerConsumerProblem)

add_library(lib_pcp_thread STATIC pcp.c)
target_compile_definitions(lib_pcp_thread PRIVATE MODE_THREAD)
target_link_libraries(lib_pcp_thread PRIVATE pthread)
add_executable(pcp_thread thread.c)
target_link_libraries(pcp_thread PRIVATE  lib_pcp_thread)

add_library(lib_pcp_process STATIC pcp.c)
target_compile_definitions(lib_pcp_process PRIVATE MODE_PROCESS)
target_link_libraries(lib_pcp_process PRIVATE pthread)
add_executable(pcp_process process.c)
target_link_libraries(pcp_process PRIVATE lib_pcp_process)

