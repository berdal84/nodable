# Nodable's main CMakeLists.txt

# prerequisites:
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version") # earliest version supporting c++11

# project declaration
project( "Nodable" VERSION "0.8.0" )
option( ENABLE_TESTS "Enable tests" on )

# define where outputs goes
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)

# include compiler configuration script
include( cmake/compiler-config.cmake )

# If a default install prefix has not been set, we set it to ./bin
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/bin" CACHE PATH "..." FORCE)

# libraries (external)
add_subdirectory( ./libs )

# nodable
add_subdirectory( ./src )

# Unit tests (gtest)
if ( ENABLE_TESTS )
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
    add_subdirectory( tests )
endif()
