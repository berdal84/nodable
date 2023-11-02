# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.
include(../../../scripts/cmake/log.cmake)
ndbl_log_title_header()

add_library(
    nodable-core
    STATIC
        Component.cpp
        ComponentBag.cpp
        IfNode.cpp
        DataAccess.cpp
        DirectedEdge.cpp
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
        SlotRef.cpp
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

add_executable(
        test-nodable-core
        Graph.specs.cpp
        Node.specs.cpp
        Property.specs.cpp
        Slot.specs.cpp
        Token.specs.cpp
        language/Nodlang.specs.cpp
        language/Nodlang.tokenize.specs.cpp
        language/Nodlang.parse_function_call.specs.cpp
        language/Nodlang.parse_token.specs.cpp
        VirtualMachine.specs.cpp
        language/Nodlang.parse_and_eval.specs.cpp
        )
target_link_libraries(test-nodable-core PRIVATE gtest_main gtest nodable-core )
add_test( NAME test_nodable_core COMMAND test-nodable-core )

# Benchmarks
#-----------

# Nodlang
add_executable( bench-nodable-core-Nodlang language/Nodlang.bench.cpp )
target_link_libraries( bench-nodable-core-Nodlang PRIVATE benchmark::benchmark nodable-core framework-core )

