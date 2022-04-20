# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.
include(${PROJECT_SOURCE_DIR}/cmake/log.cmake)
nodable_log_title_header()

# checks
nodable_message("Checking ...")
if( NOT NODABLE_CORE_INSTALL_DESTINATION)
    nodable_warn(" - Variable NODABLE_CORE_INSTALL_DESTINATION not defined, default value will be used.")
endif()

# options
option(NODABLE_CORE_INSTALL_ENABLE      "Enable nodable-core static library install" ON )
option(NODABLE_CORE_INSTALL_DESTINATION "Install folder"                             dev/nodable-core )

# log options
nodable_message("Logging options ...")
nodable_message(" - NODABLE_CORE_INSTALL_ENABLE:      ${NODABLE_CORE_INSTALL_ENABLE}")
nodable_message(" - NODABLE_CORE_INSTALL_DESTINATION: ${NODABLE_CORE_INSTALL_DESTINATION}")

add_library(
    nodable-core
    STATIC
        src/Component.cpp
        src/ConditionalStructNode.cpp
        src/DataAccess.cpp
        src/ForLoopNode.cpp
        src/GraphNode.cpp
        src/InstructionNode.cpp
        src/InvokableComponent.cpp
        src/LiteralNode.cpp
        src/Log.cpp
        src/Member.cpp
        src/NodableParser.cpp
        src/NodableSerializer.cpp
        src/Node.cpp
        src/NodeFactory.cpp
        src/Properties.cpp
        src/Scope.cpp
        src/String.cpp
        src/System.cpp
        src/Token.cpp
        src/TokenRibbon.cpp
        src/VariableNode.cpp
        src/VirtualMachine.cpp
        src/Way.cpp
        src/Wire.cpp
        src/assembly/Code.cpp
        src/assembly/Compiler.cpp
        src/assembly/Instruction.cpp
        src/reflection/qword.cpp
        src/languages/NodableLanguage.cpp
        src/languages/NodableLibrary_biology.cpp
        src/languages/NodableLibrary_math.cpp
        src/reflection/func_type.cpp
        src/reflection/type.cpp
        src/reflection/type_register.cpp
        src/reflection/variant.cpp)

target_link_libraries(
    nodable-core
    PUBLIC # <-------------- Those will be accessible by other targets if they link nodable-core
        ghc_filesystem     # c++ compatible std::filesystem::path like API
        Observe            # event system using observer/observable pattern.
    PRIVATE
        whereami           # get absolute executable/dll directory path
        rapidjson          # read/write json
)

target_include_directories(
    nodable-core
    PUBLIC
        include/
)

if( ${NODABLE_CORE_INSTALL_ENABLE} )
    nodable_message("Defining install ...")
    install( TARGETS   nodable-core ARCHIVE DESTINATION "${NODABLE_CORE_INSTALL_DESTINATION}")
    install( DIRECTORY include/             DESTINATION "${NODABLE_CORE_INSTALL_DESTINATION}lib/include" FILES_MATCHING PATTERN "*.h")
else()
    nodable_message("Skip install")
endif()
