
# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.

# Declare our main executable
add_executable(
    nodable_tests
    src/main.cpp
    src/specs/Function.specs.cpp
    src/specs/GraphNode.specs.cpp
    src/specs/Language.specs.cpp
    src/specs/Member.specs.cpp
    src/specs/Node.specs.cpp
    src/specs/Parser.specs.cpp
    src/specs/Reflect.specs.cpp
    src/specs/Semantic.specs.cpp
    src/specs/VM.specs.cpp
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