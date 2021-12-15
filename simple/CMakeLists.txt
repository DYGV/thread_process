cmake_minimum_required(VERSION 3.1)
enable_language(C)
project(simple)

add_executable(thread_lock thread_lock.c)
target_link_libraries(thread_lock PRIVATE pthread)

add_executable(process_lock process_lock.c)

add_executable(process_lock_posix_ver process_lock_posix_ver.c)
target_link_libraries(process_lock_posix_ver PRIVATE pthread)
