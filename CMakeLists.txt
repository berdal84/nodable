# Nodable's main CMakeLists.txt

# prerequisites:
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# project declaration
project( "Nodable" VERSION "0.8.0" )
option( ENABLE_TESTS "Enable gtest" on )

# define where outputs goes
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)

include( cmake/shared-project.config.cmake )

# If a default install prefix has not been set, we set it to ./bin
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/bin" CACHE PATH "..." FORCE)

# libraries (external)
add_subdirectory( ./libs )

# nodable
add_subdirectory( ./src )

# Unit tests (gtest)
if ( ENABLE_TESTS )
    enable_testing()
    add_subdirectory( tests )
endif()
