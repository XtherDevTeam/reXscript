# reXscript - Relaunched Xscript Project.

Yet another programming language.

**NOTICE**: The VM version of reXscript will be merged to master branch when the port works of std is done. Please make sure you code could work correctly in new version.

Here are the changes in new version:

1. The result of iterators' `next()` method:
    
    Here is the format of result: `[value, isEnd]`

    From this version on, `next()` method should return `[null, true]` when iterate is done, otherwise return `[value, false]`.

2. Changes on native ABI

    If you are a native module developer, change `rex::environment` `rex::interpreter` etc. to `rex::bytecodeEngine::[balahbalah]`. And, use `throwErr` instead of throwing a `signalException`     

## Usage

```bash
Usage: rex [--help] [-m module] [file] args

Optional arguments: 
  -m                execute the specify module 
  --generate-ffi	specify the FFI config file
  file          	specify the file to be executed
  args              the arguments pass to reXscript as `rexArgs`
```

## Build

To build for production, execute the following script to build reXscript.

```bash
make cmake_prod
make build_prod
cd cmake-build-release
./rex --version # 1.0
```

To build for debugging or developing, execute the following script to build reXscript.

```bash
make cmake
make build
cd cmake-build-debug
./rex --version # 1.0
```

# Integrate reXscript into your own CMake project

Add following script to your `CMakeFiles.txt`.

```cmake
set(CMAKE_CXX_STANDARD 20)

set(REX_IMPORT_AS_LIB "yes")

set(REX_SRC_DIR ${PROJECT_SOURCE_DIR}/reXscript)

include(${REX_SRC_DIR}/rex.cmake)

# add ${REX_SRC} to your target
# e.g. add_executable(rex exec/main.cpp ${REX_SRC})
```

# Third-party softwares

We used some third-party open source softwares to build the whole reXscript project, here is a list of them.

- [whereami](https://github.com/gpakosz/whereami) MIT License