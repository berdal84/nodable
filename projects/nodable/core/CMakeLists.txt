# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.
include(../../../cmake/log.cmake)
ndbl_log_title_header()

# checks
ndbl_log("Checking ...")
if( NOT NDBL_CORE_INSTALL_DESTINATION)
    ndbl_warn(" - Variable NDBL_CORE_INSTALL_DESTINATION not defined, default value will be used.")
endif()

# options
option(NDBL_CORE_INSTALL_ENABLE      "Enable nodable-core static library install" ON )
option(NDBL_CORE_INSTALL_DESTINATION "Install folder"                             dev/nodable-core )

# log options
ndbl_log("Logging options ...")
ndbl_log(" - NDBL_CORE_INSTALL_ENABLE:      ${NDBL_CORE_INSTALL_ENABLE}")
ndbl_log(" - NDBL_CORE_INSTALL_DESTINATION: ${NDBL_CORE_INSTALL_DESTINATION}")

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

if( ${NDBL_CORE_INSTALL_ENABLE} )
    ndbl_log("Defining install ...")
    install( TARGETS   nodable-core ARCHIVE DESTINATION "${NDBL_CORE_INSTALL_DESTINATION}")
    install( DIRECTORY include DESTINATION "${NDBL_CORE_INSTALL_DESTINATION}lib/include" FILES_MATCHING PATTERN "*.h")
else()
    ndbl_log("Skip install")
endif()
