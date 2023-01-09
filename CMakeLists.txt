# Nodable's main CMakeLists.txt

# prerequisites:
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
set(CMAKE_OSX_DEPLOYMENT_TARGET      "10.9" CACHE STRING "Minimum OS X deployment version") # must be declared BEFORE project
                                                                                            # 10.9 is earliest version supporting c++11

# project declaration
project( "Nodable" VERSION "0.9" )

# options/variables
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY   ${CMAKE_BINARY_DIR}/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY   ${CMAKE_BINARY_DIR}/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY   ${CMAKE_BINARY_DIR}/)
set(CMAKE_INSTALL_PREFIX             "${CMAKE_CURRENT_LIST_DIR}/out/app")      # ./out/ folder must not be changed, referenced in readme.md
set(NODABLE_PACKAGE_DIR              "${CMAKE_CURRENT_LIST_DIR}/out/package")  #                     //
set(NODABLE_CONFIGURED_DIR           "${PROJECT_BINARY_DIR}/configured/nodable")
set(NODABLE_SKIP_TESTS               OFF )
set(NODABLE_CORE_INSTALL_ENABLE      OFF)
set(NODABLE_CORE_INSTALL_DESTINATION libs) # relative to CMAKE_INSTALL_PREFIX

# include *.cmake scripts
include( cmake/log.cmake )
include( cmake/compiler-config.cmake )

#include other CMakeLists.txt
add_subdirectory(libs)     # libraries (SDL, lodepng, Dear ImGui, etc.)
add_subdirectory(src/core) # nodable core (static library).
add_subdirectory(src/cli)  # nodable application (terminal).
add_subdirectory(src/app)  # nodable application (with gui).
add_subdirectory(tests)

#  Packaging:
#===========

include(cmake/cpack.cmake)
