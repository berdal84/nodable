#
# The role of this file is to gather all informations about nodable
# dependencies.
#-------------------------------------------------------------------

# mirror
#-------
include( ${CMAKE_CURRENT_LIST_DIR}/mirror/mirror-config.cmake )

# imgui
#------
set(IMGUI_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui.cpp
        ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_demo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_draw.cpp
        ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_widgets.cpp
        ${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_opengl3.cpp
        ${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_sdl.cpp
        )

# imgui color text edit
#----------------------
set( IMGUI_TXTEDITOR_SOURCES ${CMAKE_CURRENT_LIST_DIR}/ImGuiColorTextEdit/TextEditor.cpp)

aux_source_directory( "${CMAKE_CURRENT_LIST_DIR}/gl3w/GL" GL3W)
aux_source_directory( "${CMAKE_CURRENT_LIST_DIR}/lodepng" LODE_PNG)

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

set( LIBS_SOURCES
        ${GL3W}
        ${IMGUI_SOURCES}
        ${IMGUI_TXTEDITOR_SOURCES}
        ${MIRROR_SOURCES}
        ${LODE_PNG}
        )

set( LIBS_INCLUDE_DIRS
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/imgui
        ${CMAKE_CURRENT_LIST_DIR}/ImGuiColorTextEdit
        ${CMAKE_CURRENT_LIST_DIR}/IconFontCppHeaders
        ${CMAKE_CURRENT_LIST_DIR}/imgui-filebrowser
        ${SDL2_INCLUDE_DIRS}
        ${CMAKE_CURRENT_LIST_DIR}/gl3w/GL
        ${CMAKE_CURRENT_LIST_DIR}/gl3w
        ${CMAKE_CURRENT_LIST_DIR}/glfw/include/GLFW
        ${CMAKE_CURRENT_LIST_DIR}/lodepng
        ${MIRROR_INCLUDE_DIRS}
    )

if( NODABLE_ENABLE_CONFIG_LOGS )
    message( "libs-config.cmake report:")
    message( "\tLIBS_SOURCES: ${LIBS_SOURCES}" )
    message( "\tLIBS_INCLUDE_DIRS: ${LIBS_INCLUDE_DIRS}" )
endif()