<img src="https://www.dalle-cort.fr/wp-content/uploads/2019/07/2019_08_04_Nodable_Logo_V2.jpg" width=600 />
   
<a href="https://github.com/berdal84/Nodable/actions?query=workflow%3Abuild" title="linux/windows x64">
<img src="https://github.com/berdal84/nodable/workflows/build/badge.svg" />
</a>

Nodable is node-able !
======================

Introduction:
-------------

This software is **a node-able bidirectionnal expression editor**.

More precisely, it means **Text-to-Node and Node-to-Text seamless edition**.

To download a binary: go to this [article](https://www.dalle-cort.fr/nodable-node-oriented-programming/). By the way, if you're interested by the architecture, the language or the history of Nodable you'll find some documentation too.

You still don't understand what I'm doing? I hope this GIF will make this more understandable:

![Demo GIF](https://www.dalle-cort.fr/wp-content/uploads/2018/01/2019_06_06_Nodable_0.4.1wip_Berenger_Dalle-Cort.gif)


How to compile ? :
==================

Requirements:
- A **C++17** compatible build system (tested with make/g++-10 and MSVC14.27.29110)
- Libraries `libsdl2-dev` and `libegl1-mesa-dev` (for linux only, win32 binaries are included)
- **CMake 3.14+**

Clone the Nodable repository (with submodules):

```
git clone https://github.com/berdal84/Nodable.git --recurse-submodules
```

Configure and run the build:

```
cd ./Nodable
cmake . -B build
cmake --build build --config Release [--target install]
```
*Optionnal `--target install` is to create a clean `./install/Release` directory with only necessary files to run the software.*

Nodable will be built into `./build/`

To run it:
```
cd build
./Nodable
```


Dependencies / Credits :
==============

- SDL2 : https://www.libsdl.org/
- GLFW3 : http://www.glfw.org/
- *Dear ImGui* developed by Omar Cornut: https://github.com/omarcornut/imgui
- IconFontCppHeaders by Juliette Faucaut: https://github.com/juliettef/IconFontCppHeaders
- ImGuiColorTextEdit by BalazsJako : https://github.com/BalazsJako/ImGuiColorTextEdit
- mirror by Grouflon : https://github.com/grouflon/mirror
- ImGui FileBrowser by AirGuanZ: https://github.com/AirGuanZ/imgui-filebrowser

Licence:
=========
**Nodable** is licensed under the GPL License, see LICENSE for more information.

Each submodule are licensed, browse */extern* folder.
