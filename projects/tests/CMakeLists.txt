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
        src/fixtures/core.h
        src/specs/graph.specs.cpp
        src/specs/language.specs.cpp
        src/specs/node.specs.cpp
        src/specs/parse_and_eval.specs.cpp
        src/specs/parse_token.specs.cpp
        src/specs/property.specs.cpp
        src/specs/reflection.specs.cpp
        src/specs/token.specs.cpp
        src/specs/tokenize.specs.cpp
        src/specs/type_cast.specs.cpp
        src/specs/virtual_machine.specs.cpp
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
        src/specs/gui.specs.cpp
)

# Link libraries
target_link_libraries(
    nodable-gui-tests
    PRIVATE
        gtest_main
        gtest
        nodable-gui
)

# Benchmarks
#-----------

# Declare our main executable
add_executable(
        nodable-core-benchmark
        src/main_benchmark.cpp
)

# Link libraries
target_link_libraries(
        nodable-core-benchmark
        PRIVATE
        benchmark::benchmark # google's
        nodable-core
        framework-core
)

# Add test executables
#---------------------
cmake_policy(SET CMP0110 NEW) # Allow spaces in test names
add_test( NAME "Nodable Core" COMMAND nodable-core-tests )

# GUI tests does not work on every machine (only MacOS in software on GitHub Actions)
if( $ENV{JETBRAINS_IDE} )
    ndbl_log("JETBRAINS_IDE is defined:  Enable Nodable GUI tests (hardware rendering)")
    add_test( NAME "Nodable GUI (hardware rendering)" COMMAND nodable-gui-tests)
    add_definitions(-D NDBL_GUI_TEST_HUMAN_SPEED )
elseif(WIN32)
    ndbl_log("Windows detected: Skip Nodable GUI tests")
elseif(APPLE)
    ndbl_log("Apple detected: Enable Nodable GUI tests (software rendering)")
    add_test( NAME "Nodable GUI (software rendering)" COMMAND nodable-gui-tests)
elseif(UNIX) # Should be tested after APPLE
    ndbl_log("Linux detected: Skip Nodable GUI tests")
endif()


