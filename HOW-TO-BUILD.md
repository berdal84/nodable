[Home](./README.md) ->  Build

# How to build?

## Architecture

Nodable is split in two [src](./src/README.md):
- the [nodable](./src/nodable/README.md) project.
- the [framework](./src/fw/README.md) project.

They both rely on external [libraries](./libs/README.md).

## Prerequisites:
- System:
  - Windows 64bits / Linux 64bits / macOS 10.13+ 
  - CMake 3.14+
  - A C++11 compatible build system*
- Libraries
    - Require OpenGL which is usually preinstalled under macOS and Windows, under Linux install mesa (ex. for Ubuntu: `sudo apt-get install libegl1-mesa`)
    - Under linux `pkg-config` and `libgtk-3-dev` are required. (ex. for Ubuntu: `sudo apt-get install pkg-config libgtk-3-dev`)
      Below, are all the commands to clone, configure, and build nodable from sources.

_*no help is available here for that particular subject_

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
- `--target install` is to create a clean `out/app` directory with only the necessary files to run the software.

