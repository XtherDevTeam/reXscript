if (DEFINED REX_IMPORT_AS_LIB)
    file(GLOB REX_SRC
            ${REX_SRC_DIR}/frontend/*.cpp
            ${REX_SRC_DIR}/backend/*.cpp
            ${REX_SRC_DIR}/interpreter/*.cpp
            ${REX_SRC_DIR}/pass/*.cpp
            ${REX_SRC_DIR}/share/*.cpp
            ${REX_SRC_DIR}/share/whereami/*.cpp
            ${REX_SRC_DIR}/exceptions/*.cpp
            ${REX_SRC_DIR}/ffi/*.cpp
            ${REX_SRC_DIR}/*.cpp)

    include_directories(${REX_SRC_DIR})
endif ()

if (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    set(CMAKE_CXX_FLAGS "-ldl -lpthread")
endif ()


if (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    set(CMAKE_CXX_FLAGS "-static -lpthread")
endif ()