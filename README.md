# mizuna-utils

this repository contains various scripts for interacting with some ActionLibrary games (e.g. SMO, SM3DW).

## Building

There is a workflow that uploads automatic builds [here](https://github.com/tetraxile/mizuna-utils/actions/workflows/build.yml) after every commit.

### Linux

* `git submodule update --init --recursive`
* `cmake -B build --toolchain=cmake/toolchain.cmake`
* `cmake --build build`

### Windows

* `git submodule update --init --recursive`
* `mkdir build`
* `cd build`
* `cmake .. -G "Visual Studio 17 2022" -A x64 && cmake --build . --config Release`

## Usage

### al-search

```
usage: ./al-search [game] [options...]

options:
	-r, --romfs    path to game's romfs
	-n, --name     name of object to search for
	-o, --output   path to output file
```

this script searches through all of a game's stages for an object that matches the search criteria.

`game` can currently only be `smo` and `3dw`, for Odyssey and 3D World respectively.

`object name` matches a `UnitConfigName`, `ModelName`, or `ParameterConfigName`.

### mizuna-utils

```
usage: ./mizuna-utils <format> <options...>
        formats: yaz0, sarc, szs, byml, msbp
```

various readers/writers for different file formats

### al-config

```
usage: ./al-config <subcommand> <args...>

./al-config romfs <game> <romfs path>
```

set config options for the other scripts to use

## License

The licenses found in the [LICENSE](LICENSE) file apply only to the source files in the [src/](src) directory.

## Credits

* [mINI](https://github.com/metayeti/mINI) for .ini file parsing
* [argspp](https://github.com/dmulholl/argspp) for command line argument parsing
* [zstd](https://github.com/facebook/zstd) for interacting with ZSTD compressed archives
