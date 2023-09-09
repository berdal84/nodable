# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.
include(../../../cmake/log.cmake)
ndbl_log_title_header()

add_library(
    nodable-core
    STATIC
        Component.cpp
        Components.cpp
        ConditionalStructNode.cpp
        DataAccess.cpp
        Edge.cpp
        ForLoopNode.cpp
        Graph.cpp
        GraphUtil.cpp
        InstructionNode.cpp
        InvokableComponent.cpp
        LiteralNode.cpp
        Node.cpp
        NodeFactory.cpp
        Property.cpp
        PropertyBag.cpp
        Scope.cpp
        Slot.cpp
        SlotBag.cpp
        Token.cpp
        TokenRibbon.cpp
        VariableNode.cpp
        VirtualMachine.cpp
        WhileLoopNode.cpp
        assembly/Code.cpp
        assembly/Compiler.cpp
        assembly/Instruction.cpp
        language/Nodlang.cpp
        language/Nodlang_biology.cpp
        language/Nodlang_math.cpp
)

target_link_libraries(
    nodable-core
    PUBLIC # <-------------- Those will be accessible by other targets if they link nodable-core
        framework-core
)

target_include_directories(
    nodable-core
    PUBLIC
        ../../../src
        ../../nodable
)

ndbl_log("Defining install ...")
set_target_properties(nodable-core PROPERTIES OUTPUT_NAME "core" )

#install( TARGETS   nodable-core ARCHIVE DESTINATION "libs")
#install( DIRECTORY include DESTINATION "libs" FILES_MATCHING PATTERN "*.h")

ndbl_log_title_header()

# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.

ndbl_log("NDBL_SKIP_TESTS: ${NDBL_SKIP_TESTS}")

if ( NDBL_SKIP_TESTS )
    return()
endif()

# Headless tests (nodable-core)
#------------------------------

# Declare our main executable
add_executable(
        nodable-core-tests
        Graph.specs.cpp
        Node.specs.cpp
        Property.specs.cpp
        Token.specs.cpp
        VirtualMachine.specs.cpp
        fixtures/core.h
        language/Nodlang.parse_and_eval.specs.cpp
        language/Nodlang.parse_function_call.specs.cpp
        language/Nodlang.parse_token.specs.cpp
        language/Nodlang.specs.cpp
        language/Nodlang.tokenize.specs.cpp
        )

# Link libraries
target_link_libraries(
        nodable-core-tests
        PRIVATE
        gtest_main
        gtest
        nodable-core
)

# Benchmarks
#-----------

# Declare our main executable
add_executable(
        nodable-core-benchmark
        benchmark.cpp
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

