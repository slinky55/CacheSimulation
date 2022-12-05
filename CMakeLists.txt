cmake_minimum_required(VERSION 3.15)

project(Cache
        LANGUAGES CXX
        VERSION 0.1.0
        DESCRIPTION "Cache simulation project in C++")

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} main.cpp)
