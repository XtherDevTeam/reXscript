# reXscript - Relaunched Xscript Project.

Yet another programming language.

## Usage

```bash
Usage: rex [--help] [--version] [--shell VAR] [--args VAR...] [--generate-ffi VAR] file

Positional arguments:
  file          	specify the file to be executed 

Optional arguments:
  -h, --help    	shows help message and exits 
  -v, --version 	prints version information and exits 
  --shell       	open interactive shell 
  --args        	specify the arguments to pass to the reXscript program [nargs=1..1145141919] 
  --generate-ffi	specify the FFI config file 
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

## Third-party projects

We used some third-party opensource projects in reXscript's developing.

Here is a list of them.

Format: `[projectName (author) (license)](link) purpose`

- [argparse (p-ranav) (MIT License)](https://github.com/p-ranav/argparse) for arguments parsing
  for charset conversation between std::string and std::wstring 