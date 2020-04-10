# CMakeList.txt : fichier projet CMake de niveau supérieur, effectuez une configuration globale
# et incluez les sous-projets ici.
#
cmake_minimum_required (VERSION 3.8)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project ("Nodable")

find_package(OpenGL REQUIRED)

# Set SOURCES
aux_source_directory( "./sources/" NODABLE)
aux_source_directory( "./extern/imgui" IMGUI)
aux_source_directory( "./extern/ImGuiColorTextEdit" TEXTEDIT)
aux_source_directory( "./libs/gl3w/GL" GL3W)

set(SOURCES
  "./sources/main"
  ${NODABLE}
  ${GL3W}
  ${IMGUI}
  ${TEXTEDIT}    
  "./extern/imgui/examples/imgui_impl_opengl3.cpp"
  "./extern/imgui/examples/imgui_impl_sdl.cpp"
)

include_directories(
    "./extern"
    "./extern/imgui"
    "./extern/ImGuiColorTextEdit"
    "./extern/IconFontCppHeaders"
    "./libs"
    "./libs/SDL2/include"
    "./libs/gl3w/GL"
    "./libs/gl3w"
    "./libs/glfw/include/GLFW"
)

# Ajoutez une source à l'exécutable de ce projet.
add_executable (Nodable ${SOURCES})

# Set LIBS
set(SDL2
  "./libs/SDL2/lib/x64/SDL2"
  "./libs/SDL2/lib/x64/SDL2main"
)
target_link_libraries(Nodable PRIVATE ${SDL2})

target_link_libraries(Nodable PRIVATE ${OPENGL_LIBRARY})






# TODO: Ajoutez des tests et installez des cibles si nécessaire.

