cmake_minimum_required(VERSION 3.13)
project(telco)

set(CMAKE_C_STANDARD 99)

add_definitions(-D _REENTRANT)
add_executable(telco main.c)
target_link_libraries(telco pthread)
target_link_libraries(telco rt)
