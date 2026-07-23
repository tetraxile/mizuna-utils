.PHONY : all clean

all:
	cmake -B build --toolchain=cmake/toolchain.cmake
	cmake --build build
	./install.sh

clean:
	rm -rf build


