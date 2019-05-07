cmake_minimum_required(VERSION 3.13)
project(commando)

set(CMAKE_C_STANDARD 99)

add_definitions(-D _REENTRANT)
add_executable(commando main.c)
target_link_libraries(commando pthread)
target_link_libraries(commando rt)
