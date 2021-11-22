
# Do not use this CMaleLists.txt directly, take the one from base directory.
#---------------------------------------------------------------------------

# App
#====

file( GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp )
set( LIBS_DIR ../../libs )

# add sources
add_library(
        nodable_app
        STATIC
            ${SOURCES}
            ${LIBS_DIR}/imgui/backends/imgui_impl_opengl3.cpp
            ${LIBS_DIR}/imgui/backends/imgui_impl_sdl.cpp
            )

# Include external and libs folders (for headers)
target_include_directories(
        nodable_app
        PUBLIC
            include/
            ${PROJECT_BINARY_DIR}/conf
            ${LIBS_INCLUDE_DIRS} )

# Linking
find_package(Threads REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
if (NOT OpenGL_FOUND)
    message(FATAL_ERROR "OpenGL not found")
endif()

IF (WIN32)
    target_link_libraries(nodable_app LINK_PUBLIC ${SDL2_STATIC} )
ELSE()
    target_link_libraries(nodable_app LINK_PUBLIC ${SDL2_LIBRARIES})
ENDIF()

set( NODABLE_STATIC_LIBS
        Threads::Threads
        imgui
        imgui_textedit
        IconFontCppHeaders
        lodepng
        gl3w
        nodable_core
        ${CMAKE_DL_LIBS}
        OpenGL::GL
        ghc_filesystem
        )

IF (APPLE)
    find_library(APPLE_CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    find_library(APPLE_COCOA_FRAMEWORK Cocoa)
    find_library(APPLE_CARBON_FRAMEWORK Carbon)
    set( NODABLE_STATIC_LIBS
            ${NODABLE_STATIC_LIBS}
            ${APPLE_CORE_FOUNDATION_FRAMEWORK}
            ${APPLE_CARBON_FRAMEWORK}
            ${APPLE_COCOA_FRAMEWORK} )
ENDIF()

target_link_libraries(nodable_app PUBLIC ${NODABLE_STATIC_LIBS})

# Main
#=====

add_executable( nodable ./src/main.cpp )
target_link_libraries( nodable PUBLIC nodable_app ghc_filesystem)

# required to run well on recent os (ex: ubuntu)
set_property(TARGET nodable PROPERTY POSITION_INDEPENDENT_CODE FALSE)

# fix working directory for visual studio
set_property(TARGET nodable PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${BUILD_PATH}")

# AFTER BUILD:
#=============

# Copy assets (fonts, scripts, etc..)
add_custom_command(
        TARGET nodable_app PRE_BUILD
        COMMAND ${CMAKE_COMMAND}
        -E copy_directory
        "${NODABLE_APP_ASSETS_ABSOLUTE_DIR}"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NODABLE_APP_ASSETS_DIR}"
)

# Copy SDL2.dll
if(WIN32)
    add_custom_command(
            TARGET nodable_app PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${SDL2_RUNTIME} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    )
endif()

# Installation :
#===============

set(INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/app/$<CONFIG>)

# Copy DLLs
install( FILES     ${SDL2_RUNTIME}                    DESTINATION ${INSTALL_PATH} )

# Copy assets
install( DIRECTORY ${NODABLE_APP_ASSETS_ABSOLUTE_DIR} DESTINATION ${INSTALL_PATH} )
install( FILES     ${PROJECT_SOURCE_DIR}/CHANGELOG.md DESTINATION ${INSTALL_PATH} )
install( FILES     ${PROJECT_SOURCE_DIR}/README.md    DESTINATION ${INSTALL_PATH} )

# Copy executable
install( TARGETS   nodable RUNTIME                    DESTINATION ${INSTALL_PATH} )
