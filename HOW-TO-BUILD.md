[Home](./README.md) ->  Build

# How to build?

## Architecture

Nodable sources are split in two folders under [./src](./src/README.md):
- [./src/ndbl](src/ndbl/README.md) project.
- [./src/tool](src/tools/README.md) project.

They both rely on external [libraries](./libs/README.md).

## Prerequisites:

### System
- Windows 64bits
- Linux 64bits (only Ubuntu is tested)
- macOS 13+

### Software
- CMake 3.14 and above
- a C++17 compiler

### Libraries (for Linux ONLY)

From a terminal, run:
```
sudo apt-get install libegl1-mesa pkg-config libgtk-3-dev libasound2-dev
```

## Build commands

Run the following commands:
```console
git clone --branch v0.9.11 https://github.com/berdal84/nodable.git --recurse-submodules
cd nodable
cmake . -B cmake-build-there
cmake --build cmake-build-there --config Release --target install
```
Once all commands are succeeded you must see a new folder `out` containing a folder `app`, inside there is all you need to run *Nodable*.
On Windows execute: `nodable.exe`, on Linux and macOS run `./nodable`.

Few details about the commands above:

- `--recurse-submodules` is important when cloning since *Nodable* needs other git repositories to be built.
- `--branch v<major>.<minor>.<patch>` is to target a specific tag, it is recommended to get a stable version. You can try a more recent if you wish. Browse [tags list](https://github.com/berdal84/nodable/tags).
- `--target install` is to create_new a clean `out/app` directory with only the necessary files to run the software.

