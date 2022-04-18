
# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.

# Declare our main executable
add_executable(
    nodable_tests
    src/main.cpp
        src/specs/graph.specs.cpp
        src/specs/language.specs.cpp
        src/specs/member.specs.cpp
        src/specs/node.specs.cpp
        src/specs/parser.specs.cpp
        src/specs/reflection.specs.cpp
        src/specs/virtual_machine.specs.cpp
    src/specs/type_cast.specs.cpp
)

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