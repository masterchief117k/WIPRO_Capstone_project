cmake_minimum_required(VERSION 3.16)
project(assignment_1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(assignment_1
    src/main.cpp
    src/commands.cpp
    src/fsops.cpp
    src/permissions.cpp
    src/search.cpp
    src/util.cpp
)

target_include_directories(assignment_1 PRIVATE include)

