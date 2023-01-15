
# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.

if ( NDBL_SKIP_TESTS )
    nodable_message("tests skipped")
    return()
endif()

# gtest
#-------
enable_testing()
# https://stackoverflow.com/questions/12540970/how-to-make-gtest-build-mdd-instead-of-mtd-by-default-using-cmake
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# we don't need mocks (for now) nor install
option(BUILD_GMOCK OFF)
option(INSTALL_GTEST OFF)
add_subdirectory(libs/googletest)

# nodable tests
#--------------

# Declare our main executable
add_executable(
    nodable_tests
    src/main.cpp
        src/specs/graph.specs.cpp
        src/specs/language.specs.cpp
        src/specs/property.specs.cpp
        src/specs/node.specs.cpp
        src/specs/parser.specs.cpp
        src/specs/reflection.specs.cpp
        src/specs/virtual_machine.specs.cpp
    src/specs/type_cast.specs.cpp
        src/specs/token.specs.cpp)

# Link libraries
target_link_libraries(
    nodable_tests
    PRIVATE
        gtest_main
        gtest
        nodable-core
)

# declare a single test
add_test( NAME nodable_tests COMMAND nodable_tests )