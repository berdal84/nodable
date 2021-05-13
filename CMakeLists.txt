
#
# This file is the main CMakeLists.txt, it can build nodable, nodable_lib or tests
#---------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.14)
project( "Nodable Meta" VERSION "0.8.0" )

option( ENABLE_TESTS "Enable gtest" on )
option( NODABLE_ENABLE_CONFIG_LOGS "enable/disable configuration logs (for included scripts)" off)

#---------------------------------------------------------------------------------

if ( NOT DEFINED CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE Release)
endif()

# include helpers for libs
include( nodable/lib/lib-config.cmake )
include( libs/libs-config.cmake )

# Get architecture
if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
    set(ARCHITECTURE "x64")
else ()
    set(ARCHITECTURE "x86")
endif ()
include(CheckPIESupported)
check_pie_supported() # avoid "unable to run shared libraries" under linux

# specify the C++ standard
set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        ON)

# enable threads
set(THREADS_PREFER_PTHREAD_FLAG ON)

# define _DEBUG (MSVC define it under windows but this is not the default behavior
# with other compilers)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

#----------------------------------------------------------------------------------

# If a default install prefix has not been set, we set it.
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/bin" CACHE PATH "..." FORCE)

# Nodable (both app and library)
#-------------------------------
add_subdirectory( nodable )

# Unit tests (gtest)
#-------------------
if ( ENABLE_TESTS )
    enable_testing()
    add_subdirectory( tests )
endif()

#--------------------------------------------------------------------------------

# Group sources in a tree (mainly for VisualStudio)
source_group(TREE ".." FILES ${NODABLE_SOURCES})