ndbl_log_title_header()

# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.

ndbl_log("NDBL_SKIP_TESTS: ${NDBL_SKIP_TESTS}")

if ( NDBL_SKIP_TESTS )
    return()
endif()

# gtest
#-------

# https://stackoverflow.com/questions/12540970/how-to-make-gtest-build-mdd-instead-of-mtd-by-default-using-cmake
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# we don't need mocks (for now) nor install
option(BUILD_GMOCK OFF)
option(INSTALL_GTEST OFF)
add_subdirectory(libs/googletest)

# Headless tests (nodable-core)
#------------------------------

# Declare our main executable
add_executable(
    nodable-core-tests
        src/specs/graph.specs.cpp
        src/specs/language.specs.cpp
        src/specs/property.specs.cpp
        src/specs/node.specs.cpp
        src/specs/parser.specs.cpp
        src/specs/reflection.specs.cpp
        src/specs/virtual_machine.specs.cpp
        src/specs/type_cast.specs.cpp
        src/specs/token.specs.cpp
        src/fixtures/core.h
)

# Link libraries
target_link_libraries(
    nodable-core-tests
    PRIVATE
        gtest_main
        gtest
        nodable-core
)

# GUI tests (nodable-gui)
#------------------------

# Declare our main executable
add_executable(
    nodable-gui-tests
    src/specs/app.specs.cpp
)

# Link libraries
target_link_libraries(
    nodable-gui-tests
    PRIVATE
        gtest_main
        gtest
        nodable-gui
)

# Add test executables
#---------------------
cmake_policy(SET CMP0110 NEW) # Allow spaces in test names
add_test( NAME "Nodable Core" COMMAND nodable-core-tests )
IF(NOT DEFINED $ENV{CI})
    # For now, I didn't setup something to be able to run those tests in GitHub Actions.
    add_test( NAME "Nodable GUI" COMMAND nodable-gui-tests)
endif()

