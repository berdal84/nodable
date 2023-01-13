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

The goal of *Nodable* is:
- to **provide an original hybrid source code editor**, using both textual and nodal paradigms.
- to **provide happiness to the developer**, by learning language related stuff (parsers, AST, compilation, etc.)

The goal of *Nodable* is **not**:
- To handle more than 1 language
- To replace an IDE
- To be a production tool. That's why I implement things on my own instead of using proven libraries (ex: llvm), remember my second goal.

In Nodable, the textual and nodal points of view are strongly linked, in both ways:

- A change to the source code will update the graph.
- A change to the graph will update* the source code.

_*: Nodable persistant state is the source code. Some minor changes on the graph, like changing their position or adding orphaned nodes, might not update the source code_


### Screenshots

![nodable-01](https://user-images.githubusercontent.com/942052/161857692-97786562-c30c-470c-9e07-62b240a4a222.gif)

![nodable-02](https://user-images.githubusercontent.com/942052/161857699-eedb1c42-2b49-4bea-8da7-20f1b522cf73.gif)

## How to use?

Follow the instruction from the [latest release](https://github.com/berdal84/Nodable/releases/latest) section.
If you want to build *Nodable* from sources, follow [HOW TO BUILD](./HOWTOBUILD.md) instructions.

## License:

**Nodable** is licensed under the GPL License, see [LICENSE](./LICENSE) file for more information.

*Note: each submodule is licensed, browse [./libs](./libs) folder to know more.

## Credits :

**Nodable** is developped by [@berdal84](https://github.com/berdal84)

#### Libraries

| Library  | Author(s)  |
| :---     |   :---     |
| [*Dear ImGui*]( https://github.com/omarcornut/imgui) | Omar Cornut
| [*IconFontCppHeaders*](https://github.com/juliettef/IconFontCppHeaders) | Juliette Foucaut and Doug Binks
| [*ImGuiColorTextEdit*](https://github.com/BalazsJako/ImGuiColorTextEdit) | BalazsJako
| [*LodePNG*]( https://github.com/lvandeve/lodepng) | Lode Vandevenne
| [*Native file dialog extended*](https://github.com/btzy/nativefiledialog-extended) | Bernard Teo, Michael Labbe and other
| [*Observe*]( https://github.com/TheLartians/Observe) | Lars Melchior
| [*RTTR**](https://github.com/rttrorg/rttr) | Axel Menzel
| [*SDL2*](https://www.libsdl.org/) | cf. website
| [*Where am I?*](https://github.com/gpakosz/whereami.git) | Gregory Pakosz
| [*gl3w*](https://github.com/skaslev/gl3w) | cf. website
| [*googletest*](https://github.com/google/googletest) | Google
| [*gulrak/filesystem*]()  | Steffen Schümann

*: I do not depend on *RTTR* yet, but I must cite it because I am taking a lot of ideas from it. My idea is to use it in the future without requiring too many changes.

#### Resources

| Name  | Author(s)  |
| :---  | :---       |
| [*JetBrains Mono*]( https://www.jetbrains.com/lp/mono/) | Jetbrains

## Additional Information

More information about this project on [my website](https://www.dalle-cort.fr/category/project/nodable).


