<img src="https://github.com/berdal84/Nodable/blob/master/src/app/assets/images/nodable-logo-xs.png" />
   
<a href="https://github.com/berdal84/Nodable/actions?query=workflow%3AGNU%2FLinux" title="linux">
<img src="https://github.com/berdal84/nodable/workflows/GNU%2FLinux/badge.svg" />
</a>

<a href="https://github.com/berdal84/Nodable/actions?query=workflow%3AWindows" title="windows">
<img src="https://github.com/berdal84/nodable/workflows/Windows/badge.svg" />
</a>

<a href="https://github.com/berdal84/Nodable/actions?query=workflow%3AMacOS" title="macos">
<img src="https://github.com/berdal84/nodable/workflows/MacOS/badge.svg" />
</a>

# Nodable is node-able !

## Introduction:

The goal of **Nodable** is to **provide an original hybrid source code editor**, using both textual and nodal paradigm.

In Nodable, textual and nodal point of views are strongly linked, in both ways:

- A change to the source code will update the graph.
- A change to the graph will update the source code.

![](https://www.dalle-cort.fr/wp-content/uploads/2021/11/nodable-1.gif)

![](https://www.dalle-cort.fr/wp-content/uploads/2021/11/nodable-2.gif)

More information about this project [on my website](https://www.dalle-cort.fr/category/project/nodable).

## How to try ?:

### Prerequisites:

Hardware:
- a 64 bits architecture

Software:
- Operating Systems:
  - Windows (tested under Windows 10)
  - Linux (tested under Ubuntu 20.04)
  - MacOS 10.9+ (tested under Ubuntu 10.13)
- Libraries
  - Require opengl which is usually preinstalled under macOS and Windows, under Linux install mesa (ex. for Ubuntu: `sudo apt-get install libegl1-mesa`)

_Disclaimer: Nodable is a prototype, do not expect too much from it._

### Download:

Download binaries from [Releases](https://github.com/berdal84/Nodable/releases) section.

## How to compile ?

Requirements:
- Build system:
  - **CMake 3.14+**
  - A **C++11** compatible build system
- Libraries
  - Require opengl which is usually preinstalled under macOS and Windows, under Linux install mesa (ex. for Ubuntu: `sudo apt-get install libegl1-mesa`)

Below, all the commands to: clone, configure, build nodable from sources.

```
git clone --branch v0.9.3 https://github.com/berdal84/Nodable.git --recurse-submodules
cd Nodable
cmake . -B cmake-build-there
cmake --build cmake-build-there --config Release --target install
```
After that you must see a new folder `out` containing a folder `app`, inside there is all you need to run *Nodable*.
On Windows execute: `nodable.exe`, on Linux and macOS run `./nodable`. (On Linux you might have to add execution flag to the file: `chmod +x ./nodable`

*Few details about the commands above:
`--recurse-submodules` is important when cloning since nodable need other git repositories to be built.
`--branch v<major>.<minor>.<patch>` is to target a specific tag, it is recommended to get a stable version. You can try a more recent if you wish.
`--target install` is to create a clean `out/app` directory with only necessary files to run the software.*


## Licence:

**Nodable** is licensed under the GPL License, see [`LICENSE.txt`](https://github.com/berdal84/Nodable/blob/master/LICENSE.txt) for more information.

Each submodule are licensed, browse *libs/* folder.

Credits :
---------

**Nodable** is developped by [@berdal84](https://github.com/berdal84)

- Dependencies
  - Code:
    - SDL2 : https://www.libsdl.org/
    - GLFW3 : http://www.glfw.org/
    - *Dear ImGui* developed by Omar Cornut: https://github.com/omarcornut/imgui
    - IconFontCppHeaders by Juliette Faucaut: https://github.com/juliettef/IconFontCppHeaders
    - ImGuiColorTextEdit by BalazsJako : https://github.com/BalazsJako/ImGuiColorTextEdit
    - LodePNG by Lode Vandevenne: https://github.com/lvandeve/lodepng
    - Observe by Lars Melchior: https://github.com/TheLartians/Observe
    - MPark.Variant by Michael Park: https://github.com/mpark/variant
    - Where am I? by Gregory Pakosz: https://github.com/gpakosz/whereami.git
  - Resources
    - JetBrains Mono Font: https://www.jetbrains.com/lp/mono/
