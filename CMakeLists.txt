cmake_minimum_required(VERSION 3.10)
project(reXscript)

set(CMAKE_CXX_STANDARD 20)

file(GLOB REX_SRC
        frontend/*.cpp
        backend/*.cpp
        interpreter/*.cpp
        share/*.cpp
        share/whereami/*.cpp
        exceptions/*.cpp
        ffi/*.cpp
        *.cpp)

include_directories(${PROJECT_SOURCE_DIR})

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(rex exec/main.cpp ${REX_SRC})

if (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    TARGET_LINK_LIBRARIES(rex ${CMAKE_DL_LIBS} pthread)
elseif (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    set(CMAKE_CXX_FLAGS "-static -lpthread")
endif ()