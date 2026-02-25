[Home](README.md) | Build | [Libraries](extern/README.md)

# How to build?

This document explains how to build nodable from sources, if you just want to try it you can skip these steps and browse [https://nodable.42borgata.com](https://nodable.42borgata.com).

## Folder structure

Nodable sources are split in two folders under [./src](./src/README.md):
- [./src/ndbl](src/ndbl/README.md) project.
- [./src/tools](src/tools/README.md) project.

They both rely on external [libraries](extern/README.md).

## Prerequisites:

We use a custom build system that relies on RubyRake, it supports linux and windows for x64 architecture:

### Requirements
- 64bits operating system
- LLVM v20+ (we use clang as compiler and linker)
- Ruby 3+
- [vcpkg](https://vcpkg.io/) (optional, only to make changes to the libraries)

### Under Windows
- MSVC (tested on v2026) & Build Tools
>Note: only tested on Windows11

### Under Linux
- gtk3 might be required
> Note: only tested on Ubuntu24

## Build

### Clone the source code

Run the following command:
```console
git clone --branch v1.0 https://github.com/berdal84/nodable.git
```

> Few details about the commands above:
> - `--branch v<major>.<minor>.<patch>` is to target a specific tag, it is recommended to get a stable version. You can try a more recent if you wish. Browse [tags list](https://github.com/berdal84/nodable/tags).

### Install

To install run:

```console
cd nodable
rake install
```

> _Note: The default target is "desktop", but you can build for the "web" by exporting `TARGET=web` to your environment or adding it as command line argument._

Build output should be available in `build-desktop-<arch>-<os>-release/bin`, simply run `./nodable` from this folder

### Build

Run the following commands:

```console
rake build
```

Build output should be available in `build-desktop-<arch>-<os>-debug/bin`, simply run `./nodable` from this folder

You can run rake build with additionnal flags:

```console
rake build [-- --build-type=release --target=web|desktop --verbose]
```

To know more, run `rake help`.

## Run

Once built is done, the simplest way to run nodable is:

```console
rake run
```

## Test

This command will build and run tests (in both terminal and GUI mode)

```console
rake test
```

## Dev

### Working with vcpkg (microsoft package manager)

When installing new vcpkg, make sure to use this command to install

```console
rake vcpkg
```

> IMPORTANT: This will install the packages in `./vcpkg/{OS}/` subfolder.
Those files SHOULD be commited since we decided not to rebuild the libraries often.
