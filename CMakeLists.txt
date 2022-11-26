cmake_minimum_required(VERSION 3.16)
project(reXscript)

set(CMAKE_CXX_STANDARD 20)

file(GLOB REX_SRC
        frontend/*.cpp
        interpreter/*.cpp
        pass/*.cpp
        share/*.cpp
        exceptions/*.cpp)

include_directories(${PROJECT_SOURCE_DIR})

add_executable(rex main.cpp ${REX_SRC})
