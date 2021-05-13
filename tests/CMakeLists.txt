# include helpers for libs
include( ../nodable/lib/lib-config.cmake )
include( ../libs/libs-config.cmake )

# https://stackoverflow.com/questions/12540970/how-to-make-gtest-build-mdd-instead-of-mtd-by-default-using-cmake
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# we need to set ./gtest since directory is not a subdirectory...
# this will tell cmake where to put the configured files inside
# build directory.
add_subdirectory( ../libs/googletest-release-1.10.0 ./gtest)

# Includes
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${NODABLE_INCLUDE_DIRS} ${LIBS_INCLUDE_DIRS} )

# Declare our main "Tests" executable
add_executable( nodable_tests
        src/main.cpp
        src/specs/GraphNode.specs.cpp
        src/specs/Member.specs.cpp
        src/specs/Node.specs.cpp
        src/specs/Parser.specs.cpp)

# Link libraries
target_link_libraries(nodable_tests PUBLIC nodable_lib gtest gtest_main)

# declare tests
add_test( NAME nodable_tests COMMAND nodable_tests )