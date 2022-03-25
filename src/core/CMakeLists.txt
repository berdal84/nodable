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
        src/Compiler.cpp
        src/ConditionalStructNode.cpp
        src/DataAccess.cpp
        src/ForLoopNode.cpp
        src/FunctionSignature.cpp
        src/GraphNode.cpp
        src/HeadlessNodeFactory.cpp
        src/InstructionNode.cpp
        src/InvokableComponent.cpp
        src/IScope.cpp
        src/Language.cpp
        src/LanguageNodable.cpp
        src/LiteralNode.cpp
        src/Log.cpp
        src/Member.cpp
        src/Node.cpp
        src/Parser.cpp
        src/Properties.cpp
        src/R.cpp
        src/Scope.cpp
        src/Semantic.cpp
        src/Serializer.cpp
        src/Format.cpp
        src/System.cpp
        src/Token.cpp
        src/TokenRibbon.cpp
        src/VariableNode.cpp
        src/Variant.cpp
        src/VM.cpp
        src/Way.cpp
        src/Wire.cpp)

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
