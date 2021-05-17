
# Do not use this CMaleLists.txt directly, take the one from base directory.
#---------------------------------------------------------------------------

# glob the specs
file( GLOB_RECURSE SPECS CONFIGURE_DEPENDS *.specs.cpp )

# Declare our main executable
add_executable( nodable_tests src/main.cpp ${SPECS})

# Link libraries
target_link_libraries(
        nodable_tests
        PUBLIC
            gtest_main
            gtest
            nodable_core
            # nodable_app (We are not testing the whole app, just the core).
)

# declare a single test
add_test( NAME nodable_tests COMMAND nodable_tests )