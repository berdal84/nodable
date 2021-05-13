cmake_minimum_required (VERSION 3.14)

project("Nodable" VERSION "0.8.0")

set(NODABLE_APP_ASSETS_ABSOLUTE_DIR "${CMAKE_CURRENT_LIST_DIR}/app/assets")
set(NODABLE_APP_ASSETS_RELATIVE_DIR ./assets)

option(NODABLE_ENABLE_CONFIG_LOGS "Enable log message during configuration" off)

# 1 - Global Settings :
#======================

# Configure Config.h.in
configure_file("conf/Config.h.in" "conf/Config.h" )

# This script helps to get all libs sources/headers:
include( ../libs/libs-config.cmake )

# This script helps to get all Nodable sources/headers:
include( ./lib/lib-config.cmake )

# Include external and libs folders (for headers)
include_directories( ${NODABLE_INCLUDE_DIRS} ${PROJECT_BINARY_DIR}/conf ${LIBS_INCLUDE_DIRS} )

# Nodable Library:
#=================

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)

# Declare lib with nodable and libs sources.
add_library(nodable_lib ${NODABLE_SOURCES} ${LIBS_SOURCES})

# Linking
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
target_link_libraries(nodable_lib PUBLIC ${OPENGL_LIBRARY} ${CMAKE_DL_LIBS} Threads::Threads)

IF (WIN32)
    target_link_libraries(nodable_lib LINK_PUBLIC ${SDL2_STATIC} )
ELSE()
    target_link_libraries(nodable_lib PUBLIC ${SDL2_LIBRARIES})
ENDIF()

# Nodable Application:
#=====================

# add sources
add_executable( Nodable app/src/main.cpp )

# link with lib
target_link_libraries(Nodable PRIVATE nodable_lib)

# required to run well on recent os (ex: ubuntu)
set_property(TARGET Nodable PROPERTY POSITION_INDEPENDENT_CODE FALSE)

# fix working directory for visual studio
set_property(TARGET Nodable PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${BUILD_PATH}")

# AFTER BUILD:
#=============

# Copy assets (fonts, scripts, etc..)
add_custom_command(
    TARGET Nodable PRE_BUILD
    COMMAND ${CMAKE_COMMAND}
            -E copy_directory
            ${NODABLE_APP_ASSETS_ABSOLUTE_DIR}
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NODABLE_APP_ASSETS_RELATIVE_DIR}
)

# Copy SDL2.dll
IF (WIN32)
  add_custom_command(
    TARGET Nodable PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${SDL2_RUNTIME} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
  )
ENDIF()

# Installation :
#===============

set(INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/$<CONFIG>)

# Copy DLLs
install(FILES ${SDL2_RUNTIME} DESTINATION ${INSTALL_PATH})

# Copy assets
install(DIRECTORY ${NODABLE_APP_ASSETS_ABSOLUTE_DIR} DESTINATION ${INSTALL_PATH} )
install(FILES ../CHANGELOG.md DESTINATION ${INSTALL_PATH} )
install(FILES ../README.md DESTINATION ${INSTALL_PATH} )

# Copy executable
install(TARGETS Nodable RUNTIME DESTINATION ${INSTALL_PATH})
