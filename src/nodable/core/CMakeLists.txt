# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.
include(../../../cmake/log.cmake)
ndbl_log_title_header()

add_library(
    nodable-core
    STATIC
        Component.cpp
        ComponentManager.cpp
        Components.cpp
        ConditionalStructNode.cpp
        DataAccess.cpp
        DirectedEdge.cpp
        ForLoopNode.cpp
        Graph.cpp
        InstructionNode.cpp
        InvokableComponent.cpp
        LiteralNode.cpp
        Node.cpp
        NodeFactory.cpp
        Property.cpp
        PropertyGrp.cpp
        Scope.cpp
        Token.cpp
        TokenRibbon.cpp
        VariableNode.cpp
        VirtualMachine.cpp
        Way.cpp
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
