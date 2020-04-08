# CMakeList.txt : fichier projet CMake de niveau supérieur, effectuez une configuration globale
# et incluez les sous-projets ici.
#
cmake_minimum_required (VERSION 3.8)

project ("Nodable")

# Add the source files of Nodable
aux_source_directory( "./sources/" NODABLE)
aux_source_directory( "./extern/imgui" IMGUI)
aux_source_directory( "./extern/ImGuiColorTextEdit" TEXTEDIT)
aux_source_directory( "./extern/gl3w/GL" GL3W)

set(SOURCE
  ${NODABLE}
  ${IMGUI}
  ${TEXTEDIT}
  ${GL3W}
)

link_libraries("SDL2.lib")

# Include dependencies :
include_directories( "./extern")
include_directories( "./extern/imgui")
include_directories( "./extern/ImGuiColorTextEdit")
include_directories( "./extern/IconFontCppHeaders")

include_directories( "./libs")
include_directories( "./libs/SDL2/include")
include_directories( "./libs/gl3w")

# Ajoutez une source à l'exécutable de ce projet.
add_executable (Nodable ${SOURCE} )

# TODO: Ajoutez des tests et installez des cibles si nécessaire.

