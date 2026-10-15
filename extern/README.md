[Home](../README.md) | [Build](../HOW-TO-BUILD.md) | Libraries

# Libraries (external)

Nodable relies on many dependencies, thanks to the authors!

## Credits
### Libraries

| Library                                                                              | Author(s)                            |
|:-------------------------------------------------------------------------------------|:-------------------------------------|
| [*Dear ImGui*]( https://github.com/omarcornut/imgui)                                 | Omar Cornut                          |
| [*IconFontCppHeaders*](https://github.com/juliettef/IconFontCppHeaders)              | Juliette Foucaut and Doug Binks      |
| [*ImGuiColorTextEdit*](https://github.com/BalazsJako/ImGuiColorTextEdit)             | BalazsJako                           |
| [*LodePNG*]( https://github.com/lvandeve/lodepng)                                    | Lode Vandevenne                      |
| [*Native file dialog extended*](https://github.com/btzy/nativefiledialog-extended)   | Bernard Teo, Michael Labbe and other |
| [*Observe*]( https://github.com/TheLartians/Observe)                                 | Lars Melchior                        |
| [*RTTR**](https://github.com/rttrorg/rttr)                                           | Axel Menzel                          |
| [*SDL2*](https://www.libsdl.org/)                                                    | cf. website                          |
| [*Where am I?*](https://github.com/gpakosz/whereami.git)                             | Gregory Pakosz                       |
| [*gl3w*](https://github.com/skaslev/gl3w)                                            | cf. website                          |
| [*googletest*](https://github.com/google/googletest)                                 | Google                               |
| [*gulrak/filesystem*]()                                                              | Steffen Schümann                     |
| [*Freetype* v2.13.0](https://github.com/freetype/freetype/tree/VER-2-13-0)           | cf. website                          |
| [*xxHash*](https://github.com/Cyan4973/xxHash)
_If I forgot to mention something in here, please let me know at: berenger [at] 42borgata [dot] com._

*: I do not depend on *RTTR* yet, but I plan to. So I am taking inspiration from it in my code before to switch.


### Resources

| Name                                                    | Author(s) |
|:--------------------------------------------------------|:----------|
| [*JetBrains Mono*]( https://www.jetbrains.com/lp/mono/) | Jetbrains |

## Folder structure

- `vcpkg/` contains many of the dependencies as binaries (per architecture-os triplet)
- `external/` contains additionnal dependencies as source code.