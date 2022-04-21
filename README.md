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

The goal of **Nodable** is to **provide an original hybrid source code editor**, using both textual and nodal paradigms.

In Nodable, the textual and nodal points of views are strongly linked, in both ways:

- A change to the source code will update the graph.
- A change to the graph will update the source code.

![nodable-01](https://user-images.githubusercontent.com/942052/161857692-97786562-c30c-470c-9e07-62b240a4a222.gif)

![nodable-02](https://user-images.githubusercontent.com/942052/161857699-eedb1c42-2b49-4bea-8da7-20f1b522cf73.gif)

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

Below, are all the commands to clone, configure, build nodable from sources.

```
git clone --branch v0.9 https://github.com/berdal84/Nodable.git --recurse-submodules
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

Each submodule is licensed, browse *libs/* folder.

## Credits :


**Nodable** is developped by [@berdal84](https://github.com/berdal84)

#### Libraries

| Name  | Link        | Author(s)      |
| :---        |    :----   |     :---       |
| *Dear ImGui*| https://github.com/omarcornut/imgui | Omar Cornut
| *GLFW3*     | http://www.glfw.org/
| *IconFontCppHeaders*| https://github.com/juliettef/IconFontCppHeaders | Juliette Faucaut
| *ImGuiColorTextEdit* | https://github.com/BalazsJako/ImGuiColorTextEdit | BalazsJako
| *LodePNG* | https://github.com/lvandeve/lodepng | Lode Vandevenne
| *Native file dialog extended* | https://github.com/btzy/nativefiledialog-extended | Bernard Teo, Michael Labbe and other
| *Observe* | https://github.com/TheLartians/Observe | Lars Melchior
| *RTTR** | https://github.com/rttrorg/rttr | Axel Menzel
| *SDL2*      | https://www.libsdl.org/
| *Where am I?* | https://github.com/gpakosz/whereami.git | Gregory Pakosz

*: Even if I do not depend on it yet, I must cite RTTR since I am taking a lot of ideas from it in order to use it in the future without requiring too much changes.

#### Resources

| Name  | Link        | Author(s)      |
| :---        |    :----   |     :---       |
| *JetBrains Mono*      | https://www.jetbrains.com/lp/mono/ | Jetbrains

