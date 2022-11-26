cmake:
	cmake . -B cmake-build-debug  -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=DEBUG
build:
	cd cmake-build-debug; make all -j4

clean:
	cd cmake-build-debug; make clean