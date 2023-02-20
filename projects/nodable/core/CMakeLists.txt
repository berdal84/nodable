# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.
include(../../../cmake/log.cmake)
ndbl_log_title_header()

add_library(
    nodable-core
    STATIC
        src/Component.cpp
        src/ConditionalStructNode.cpp
        src/DataAccess.cpp
        src/DirectedEdge.cpp
        src/ForLoopNode.cpp
        src/GraphNode.cpp
        src/InstructionNode.cpp
        src/InvokableComponent.cpp
        src/LiteralNode.cpp
        src/Node.cpp
        src/NodeFactory.cpp
        src/Property.cpp
        src/PropertyGrp.cpp
        src/Scope.cpp
        src/Token.cpp
        src/TokenRibbon.cpp
        src/VariableNode.cpp
        src/VirtualMachine.cpp
        src/Way.cpp
        src/assembly/Code.cpp
        src/assembly/Compiler.cpp
        src/assembly/Instruction.cpp
        src/language/Nodlang.cpp
        src/language/Nodlang_biology.cpp
        src/language/Nodlang_math.cpp)

target_link_libraries(
    nodable-core
    PUBLIC # <-------------- Those will be accessible by other targets if they link nodable-core
        framework-core
)

target_include_directories(
    nodable-core
    PUBLIC
        include
)

ndbl_log("Defining install ...")
set_target_properties(nodable-core PROPERTIES OUTPUT_NAME "core" )

#install( TARGETS   nodable-core ARCHIVE DESTINATION "libs")
#install( DIRECTORY include DESTINATION "libs" FILES_MATCHING PATTERN "*.h")
