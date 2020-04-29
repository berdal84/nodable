![Nodable Logo](https://www.dalle-cort.fr/wp-content/uploads/2019/07/2019_08_04_Nodable_Logo_V2.jpg)

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

Install Microsoft Visual Studio Community 2015+

Install CMake 3.8+

Open a command line in the Nodable folder and type:

```
cmake -B ./build
cmake --build ./build --config Release
```
Nodable will be built into `./build/Release/`

Adding optionnal `--target install` to the fourth line will create a clean `./install/Release` directory with only necessary files to run the software.

You can also open the Visual Studio solution generated into `./build` if you want.

Dependencies / Credits :
==============

- SDL2 : https://www.libsdl.org/
- GLFW3 : http://www.glfw.org/
- *Dear ImGui* developed by Omar Cornut: https://github.com/omarcornut/imgui
- IconFontCppHeaders by Juliette Faucaut: https://github.com/juliettef/IconFontCppHeaders
- ImGuiColorTextEdit by BalazsJako : https://github.com/BalazsJako/ImGuiColorTextEdit

Licence:
=========
**Nodable** is licensed under the GPL License, see LICENSE for more information.

Each submodule are licensed, browse */extern* folder.
