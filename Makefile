cmake:
	cmake . -B cmake-build-debug  -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=DEBUG -G "Unix Makefiles"
build:
	cd cmake-build-debug; make all -j4

clean:
	cd cmake-build-debug; make clean

cmake_prod:
	cmake . -B cmake-build-release  -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=RELEASE -G "Unix Makefiles"

build_prod:
	cd cmake-build-release; make all -j8

clean_prod:
	cd cmake-build-release; make clean