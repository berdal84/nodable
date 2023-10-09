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
set(NDBL_APP_NAME                    "Nodable")
set(NDBL_PACKAGE_DIR                 "${CMAKE_CURRENT_LIST_DIR}/out/package")  #                     //
set(NDBL_CONFIGURED_DIR              "${PROJECT_BINARY_DIR}/configured/ndbl")
set(NDBL_SKIP_TESTS                  OFF )

# include *.cmake scripts
include(scripts/cmake/log.cmake)
include(scripts/cmake/compiler-config.cmake)
include(scripts/cmake/set-build-ref-variable.cmake)

#include other CMakeLists.txt

if ( NOT NDBL_SKIP_TESTS)
    enable_testing()
    # https://stackoverflow.com/questions/12540970/how-to-make-gtest-build-mdd-instead-of-mtd-by-default-using-cmake
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    # we don't need mocks (for now) nor install
    option(BUILD_GMOCK OFF)
    option(INSTALL_GTEST OFF)
endif()

add_subdirectory(libs lib)                # libraries (SDL, lodepng, Dear ImGui, etc.)
add_subdirectory(src/fw/core)             # core framework
add_subdirectory(src/fw/gui)              # gui framework (ImGui based)
add_subdirectory(src/fw/gui-example)      # gui example using framework-gui
add_subdirectory(src/nodable/core)        # nodable core (static library)
add_subdirectory(src/nodable/cli)         # nodable command line interface (terminal)
add_subdirectory(src/nodable/gui)         # nodable gui (gui)
add_subdirectory(src/nodable/app)         # nodable application

#  Packaging:
#===========

include(scripts/cmake/cpack.cmake)
