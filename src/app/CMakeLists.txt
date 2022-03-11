
# Do not use this CMaleLists.txt directly, take the one from base directory.
#---------------------------------------------------------------------------

# App
#====

file( GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp )
set( LIBS_DIR ../../libs )

# Linking
find_package(Threads REQUIRED)
if (NOT Threads_FOUND)
    message(FATAL_ERROR "OpenGL not found")
endif()

find_package(OpenGL REQUIRED)
if (NOT OpenGL_FOUND)
    message(FATAL_ERROR "OpenGL not found")
endif()

set( NODABLE_STATIC_LIBS
        Threads::Threads
        imgui
        imgui_textedit
        IconFontCppHeaders
        lodepng
        gl3w
        nodable_core
        ${CMAKE_DL_LIBS}
        SDL2-static
        OpenGL::GL
        ghc_filesystem
        )

IF (APPLE)
    find_library(APPLE_CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    find_library(APPLE_COCOA_FRAMEWORK Cocoa)
    set( NODABLE_STATIC_LIBS
            ${NODABLE_STATIC_LIBS}
            ${APPLE_CORE_FOUNDATION_FRAMEWORK}
            ${APPLE_COCOA_FRAMEWORK} )
ENDIF()

# Main
#=====

add_executable(
        nodable
        ./src/main.cpp
        ${SOURCES}
        ${LIBS_DIR}/imgui/backends/imgui_impl_opengl3.cpp
        ${LIBS_DIR}/imgui/backends/imgui_impl_sdl.cpp)

target_include_directories(
        nodable
        PUBLIC
        include/
        ${PROJECT_BINARY_DIR}/conf
        ${LIBS_INCLUDE_DIRS} )

target_link_libraries( nodable PUBLIC ${NODABLE_STATIC_LIBS})

# required to run well on recent os (ex: ubuntu)
set_property(TARGET nodable PROPERTY POSITION_INDEPENDENT_CODE FALSE)

# fix working directory for visual studio
set_property(TARGET nodable PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${BUILD_PATH}")

set_property(TARGET nodable PROPERTY LINK_FLAGS "-Wl" )

# AFTER BUILD:
#=============

# Copy assets (fonts, scripts, etc..)
add_custom_command(
        TARGET nodable PRE_BUILD
        COMMAND ${CMAKE_COMMAND}
        -E copy_directory
        "${NODABLE_APP_ASSETS_ABSOLUTE_DIR}"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NODABLE_APP_ASSETS_DIR}"
)


# Installation :
#===============

set(INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/app/$<CONFIG>)

# Copy assets
install( DIRECTORY ${NODABLE_APP_ASSETS_ABSOLUTE_DIR} DESTINATION ${INSTALL_PATH} )
install( FILES     ${PROJECT_SOURCE_DIR}/CHANGELOG.md DESTINATION ${INSTALL_PATH} )
install( FILES     ${PROJECT_SOURCE_DIR}/README.md    DESTINATION ${INSTALL_PATH} )

# Copy executable
install( TARGETS   nodable RUNTIME  DESTINATION ${INSTALL_PATH} )
