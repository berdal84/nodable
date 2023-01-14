#
# Create a target per library to facilitate usage by nodable targets
#-------------------------------------------------------------------

# native file dialog - extended
#------------------------------
set(NFD_PORTAL ON)                          # use portal instead of gtk, read: https://github.com/btzy/nativefiledialog-extended#using-xdg-desktop-portal-on-linux
add_subdirectory(nativefiledialog-extended EXCLUDE_FROM_ALL) # target is nfd

# filesystem (c++11 compatible)
#------------------------------
add_subdirectory(gulrak/filesystem EXCLUDE_FROM_ALL)

# Where am I
#------------
add_library(
        whereami
        STATIC
        whereami/src/whereami.c
)

target_include_directories(
        whereami
        PUBLIC
        whereami/src
)

# imgui
#------
add_library(
        ImGui
        STATIC
            imgui/imgui.cpp
            imgui/imgui_demo.cpp
            imgui/imgui_draw.cpp
            imgui/imgui_tables.cpp
            imgui/imgui_widgets.cpp
        )

target_include_directories(
        ImGui
        PUBLIC
           ./imgui/
           ./
)

# imgui color text edit
#----------------------
add_library(
        ImGuiColorTextEdit
        STATIC
            ImGuiColorTextEdit/TextEditor.cpp
)

target_include_directories(
        ImGuiColorTextEdit
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
            ./gl3w/GL
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
)
target_include_directories(IconFontCppHeaders INTERFACE ./)

# Rapid JSON
#---------
add_library(
        rapidjson
        INTERFACE
)
target_include_directories(rapidjson INTERFACE ./)

# Observe
add_subdirectory( Observe EXCLUDE_FROM_ALL) # EXCLUDE_FROM_ALL: we don't want to install it.

# SDL2
#------
add_subdirectory( SDL EXCLUDE_FROM_ALL)

