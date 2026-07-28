.PHONY : all clean

all:
	cmake -B build --toolchain=cmake/toolchain.cmake
	cmake --build build -j7
	./install.sh

clean:
	rm -rf build


