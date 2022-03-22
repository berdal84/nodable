# Nodable's main CMakeLists.txt

# prerequisites:
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# project declaration
project( "Nodable" VERSION "0.9" )

# options/variables
set(CMAKE_OSX_DEPLOYMENT_TARGET      "10.9" CACHE STRING "Minimum OS X deployment version") # earliest version supporting c++11
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY   ${CMAKE_BINARY_DIR}/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY   ${CMAKE_BINARY_DIR}/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY   ${CMAKE_BINARY_DIR}/)
set(CMAKE_INSTALL_PREFIX             "${CMAKE_CURRENT_LIST_DIR}/bin")
set(NODABLE_CONFIGURED_DIR           "${PROJECT_BINARY_DIR}/configured/nodable")
set(NODABLE_ENABLE_TESTS             ON )
set(NODABLE_CORE_INSTALL_ENABLE      OFF)
set(NODABLE_CORE_INSTALL_DESTINATION libs) # relative to CMAKE_INSTALL_PREFIX

# include *.cmake scripts
include( cmake/log.cmake )
include( cmake/compiler-config.cmake )
include( cmake/get-git-info.cmake )

#include other CMakeLists.txt
add_subdirectory(libs)     # libraries (SDL, lodepng, Dear ImGui, etc.)
add_subdirectory(src/core) # nodable core (static library).
add_subdirectory(src/app)  # nodable application (with gui).

# Unit tests (gtest)
if ( NODABLE_ENABLE_TESTS )

    # gtest
    #-------

    enable_testing()
    # https://stackoverflow.com/questions/12540970/how-to-make-gtest-build-mdd-instead-of-mtd-by-default-using-cmake
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    # we don't need mocks (for now) nor install
    option(BUILD_GMOCK OFF)
    option(INSTALL_GTEST OFF)
    # we need to set ./gtest since directory is not a subdirectory...
    # this will tell cmake where to put the configured files inside
    # build directory.
    add_subdirectory( libs/googletest-release-1.10.0)

    # nodable tests
    #--------------

    add_subdirectory( tests )
else()
    nodable_message("Skip tests (gtest and ./tests)")
endif()

#  Packaging:
#===========

include(cmake/cpack.cmake)
