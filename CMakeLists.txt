cmake_minimum_required(VERSION 3.10)
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

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    set(CMAKE_CXX_FLAGS "-ldl -lpthread")
endif ()


if (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    set(CMAKE_CXX_FLAGS "-static -lpthread")
endif ()


add_executable(rex exec/main.cpp ${REX_SRC})
