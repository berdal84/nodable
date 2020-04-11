cmake_minimum_required (VERSION 3.8)

# Declare project name
project ("Nodable")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Adds SDL2 folder to CMAKE_PREFIX_PATH
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH}
"./libs/SDL2"
)

# If a default install prefix has not been set, we set it.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/install" CACHE PATH "..." FORCE)
endif()

# Find packages
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

# Create a SOURCES variable that contains all source files
aux_source_directory( "./sources/" NODABLE)
aux_source_directory( "./extern/imgui" IMGUI)
aux_source_directory( "./extern/ImGuiColorTextEdit" TEXTEDIT)
aux_source_directory( "./libs/gl3w/GL" GL3W)

set(SOURCES
  ${NODABLE}
  ${GL3W}
  ${IMGUI}
  ${TEXTEDIT}    
  "./extern/imgui/examples/imgui_impl_opengl3.cpp"
  "./extern/imgui/examples/imgui_impl_sdl.cpp"
)

# Include headers
include_directories(
    "./extern"
    "./extern/imgui"
    "./extern/ImGuiColorTextEdit"
    "./extern/IconFontCppHeaders"
    "./libs"
    ${SDL2_INCLUDE_DIRS}
    "./libs/gl3w/GL"
    "./libs/gl3w"
    "./libs/glfw/include/GLFW"
)

# Add an executable called Nodable(.exe on windows)
add_executable (Nodable "./sources/main" ${SOURCES})

# Link libraries
target_link_libraries(Nodable PUBLIC ${OPENGL_LIBRARY})
target_link_libraries(Nodable PUBLIC ${SDL2_LIBRARIES})

# Copy DLLs
install(FILES "./libs/SDL2/lib/x64/SDL2.dll" DESTINATION ${CMAKE_INSTALL_PREFIX} )

# Copy assets
install(DIRECTORY "data" DESTINATION ${CMAKE_INSTALL_PREFIX} )

# Copy executable
install(TARGETS Nodable DESTINATION ${CMAKE_INSTALL_PREFIX} )

