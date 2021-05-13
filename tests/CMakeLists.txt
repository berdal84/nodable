
# Do not use this CMaleLists.txt directly, take the one from base directory.
#---------------------------------------------------------------------------

# https://stackoverflow.com/questions/12540970/how-to-make-gtest-build-mdd-instead-of-mtd-by-default-using-cmake
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# we need to set ./gtest since directory is not a subdirectory...
# this will tell cmake where to put the configured files inside
# build directory.
add_subdirectory( ../libs/googletest-release-1.10.0 ./gtest)

# glob the specs
file( GLOB_RECURSE SPECS CONFIGURE_DEPENDS *.specs.cpp )

# Declare our main executable
add_executable( nodable_tests src/main.cpp ${SPECS})

target_include_directories(
        nodable_tests
        PUBLIC
            ../src/app/src/
            ../src/core/src/
)
# Link libraries
target_link_libraries(
        nodable_tests
        PUBLIC
            gtest_main
            gtest
            nodable_app
            mirror)

# declare a single test
add_test( NAME nodable_tests COMMAND nodable_tests )