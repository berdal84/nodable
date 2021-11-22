#
# Create a target per library to facilitate usage by nodable targets
#-------------------------------------------------------------------

# filesystem (c++11 compatible)
#------
add_subdirectory(gulrak/filesystem)

# imgui
#------
add_library(
        imgui
        STATIC
            imgui/imgui.cpp
            imgui/imgui_demo.cpp
            imgui/imgui_draw.cpp
            imgui/imgui_widgets.cpp
        )

target_include_directories(
        imgui
        PUBLIC
           ./imgui/
           ./
)

# imgui color text edit
#----------------------
add_library(
        imgui_textedit
        STATIC
            ImGuiColorTextEdit/TextEditor.cpp
)

target_include_directories(
        imgui_textedit
        PUBLIC
            ImGuiColorTextEdit/
            imgui/
)

# gl3w
#-----
add_library(
        gl3w
        STATIC
            ./gl3w/GL/gl3w.c
            ./gl3w/GL/gl3w.h
            ./gl3w/GL/gl3w.gcda
            ./gl3w/GL/gl3w.gcno
)

target_include_directories(
        gl3w
        PUBLIC
            ./gl3w
)

# lode_png
#---------
add_library(
        lodepng
        STATIC
            ./lodepng/lodepng.cpp
            ./lodepng/lodepng.h
        )

target_include_directories(
        lodepng
        PUBLIC
            ./lodepng
)

# IconFontCppHeaders
#---------
add_library(
        IconFontCppHeaders
        INTERFACE
            ./IconFontCppHeaders/IconsFontAwesome4.h
            ./IconFontCppHeaders/IconsFontAwesome5.h
)
target_include_directories(IconFontCppHeaders INTERFACE ./)

# variant (libc++ implem C++11 compatible)
#---------
add_subdirectory(3rdparty/variant)

# Rapid JSON
#---------
add_library(
        rapidjson
        INTERFACE
            ./rapidjson/rapidjson.h
            ./rapidjson/document.h
)
target_include_directories(rapidjson INTERFACE ./)

# Observe
add_subdirectory( ${CMAKE_CURRENT_LIST_DIR}/Observe ./Observe EXCLUDE_FROM_ALL) # EXCLUDE_FROM_ALL: we don't want to install it.

# SDL2
#------
IF (WIN32)
    # We store the SDL2 binaries inside the repo.
    find_package(SDL2 REQUIRED HINTS ${CMAKE_CURRENT_LIST_DIR}/SDL2 )
ELSE()
    find_package(SDL2 REQUIRED)
ENDIF()

# Define variables to use them from the parent CMakeLists.txt
#------------------------------------------------------------

set( LIBS_INCLUDE_DIRS
        ${SDL2_INCLUDE_DIRS}
        ${CMAKE_CURRENT_LIST_DIR}/glfw/include/
        ${CMAKE_CURRENT_LIST_DIR}/gl3w/GL
        ${CMAKE_CURRENT_LIST_DIR}/gl3w
        ${CMAKE_CURRENT_LIST_DIR}/glfw/include/GLFW

        PARENT_SCOPE )
