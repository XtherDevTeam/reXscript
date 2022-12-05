cmake_minimum_required(VERSION 3.16)
project(reXscript)

set(CMAKE_CXX_STANDARD 20)

file(GLOB REX_SRC
        frontend/*.cpp
        backend/*.cpp
        interpreter/*.cpp
        pass/*.cpp
        share/*.cpp
        exceptions/*.cpp
        *.cpp)

include_directories(${PROJECT_SOURCE_DIR})

add_executable(rex exec/main.cpp ${REX_SRC})
