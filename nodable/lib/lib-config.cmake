#----------------------------------------------------------------------
# nodable.cmake script will declare header and source files so the
# caller will get the variables defined my set( ... ).
#

option(NODABLE_ENABLE_CONFIG_LOGS "Enable log message during configuration" off)

if ( NODABLE_ENABLE_CONFIG_LOGS )
 message( "nodable-config.cmake processing...")
endif()

# 2 - Script:
#============

# get file dir, to avoid relative path problems when parent call this script.
set(BASE_DIR  ${CMAKE_CURRENT_LIST_DIR})

# note: not convinced by this folders, Q: I really need folders ?
set(UTILS_DIR      ${BASE_DIR}/Utils)
set(NODE_DIR       ${BASE_DIR}/Node)
set(LANGUAGE_DIR   ${BASE_DIR}/Language)
set(CORE_DIR       ${BASE_DIR}/Core)
set(COMPONENT_DIR  ${BASE_DIR}/Component)

# 2.a - Declare source files:
#----------------------------
# note: we avoid a file() with GLOB_RECURSE since it do not detect new files. Instead we add entries manually
#       Though, our IDE can help with that task (ex: CLION does it).
set( NODABLE_SOURCES
        ${COMPONENT_DIR}/ApplicationView.cpp
        ${COMPONENT_DIR}/ComputeFunction.cpp
        ${COMPONENT_DIR}/DataAccess.cpp
        ${COMPONENT_DIR}/FileView.cpp
        ${COMPONENT_DIR}/GraphNodeView.cpp
        ${COMPONENT_DIR}/History.cpp
        ${COMPONENT_DIR}/NodeView.cpp
        ${COMPONENT_DIR}/View.cpp
        ${COMPONENT_DIR}/WireView.cpp
        ${CORE_DIR}/Application.cpp
        ${CORE_DIR}/File.cpp
        ${CORE_DIR}/Log.cpp
        ${CORE_DIR}/Member.cpp
        ${CORE_DIR}/Properties.cpp
        ${CORE_DIR}/Runner.cpp
        ${CORE_DIR}/Settings.cpp
        ${CORE_DIR}/System.cpp
        ${CORE_DIR}/Token.cpp
        ${CORE_DIR}/Variant.cpp
        ${CORE_DIR}/Way.cpp
        ${CORE_DIR}/Wire.cpp
        ${LANGUAGE_DIR}/Common/Function.cpp
        ${LANGUAGE_DIR}/Common/Language.cpp
        ${LANGUAGE_DIR}/Common/LanguageFactory.cpp
        ${LANGUAGE_DIR}/Common/Parser.cpp
        ${LANGUAGE_DIR}/Common/Semantic.cpp
        ${LANGUAGE_DIR}/Common/Serializer.cpp
        ${LANGUAGE_DIR}/Common/TokenRibbon.cpp
        ${LANGUAGE_DIR}/Nodable/NodableLanguage.cpp
        ${LANGUAGE_DIR}/Nodable/NodableParser.cpp
        ${LANGUAGE_DIR}/Nodable/NodableSerializer.cpp
        ${NODE_DIR}/AbstractCodeBlockNode.cpp
        ${NODE_DIR}/CodeBlockNode.cpp
        ${NODE_DIR}/ConditionalStructNode.cpp
        ${NODE_DIR}/DefaultNodeFactory.cpp
        ${NODE_DIR}/GraphNode.cpp
        ${NODE_DIR}/GraphTraversal.cpp
        ${NODE_DIR}/InstructionNode.cpp
        ${NODE_DIR}/LiteralNode.cpp
        ${NODE_DIR}/Node.cpp
        ${NODE_DIR}/ProgramNode.cpp
        ${NODE_DIR}/ScopedCodeBlockNode.cpp
        ${NODE_DIR}/VariableNode.cpp
        ${UTILS_DIR}/ImGuiEx.cpp
        )

# 2.b - Declare header files:
#----------------------------
# note: see source file note.
set( NODABLE_HEADERS
        ${COMPONENT_DIR}/ApplicationView.h
        ${COMPONENT_DIR}/Component.h
        ${COMPONENT_DIR}/ComputeBase.h
        ${COMPONENT_DIR}/ComputeBinaryOperation.h
        ${COMPONENT_DIR}/ComputeFunction.h
        ${COMPONENT_DIR}/ComputeUnaryOperation.h
        ${COMPONENT_DIR}/DataAccess.h
        ${COMPONENT_DIR}/FileView.h
        ${COMPONENT_DIR}/GraphNodeView.h
        ${COMPONENT_DIR}/History.h
        ${COMPONENT_DIR}/NodeView.h
        ${COMPONENT_DIR}/View.h
        ${COMPONENT_DIR}/WireView.h
        ${CORE_DIR}/Application.h
        ${CORE_DIR}/Config.h.in
        ${CORE_DIR}/File.h
        ${CORE_DIR}/Log.h
        ${CORE_DIR}/Member.h
        ${CORE_DIR}/Nodable.h
        ${CORE_DIR}/Properties.h
        ${CORE_DIR}/Runner.h
        ${CORE_DIR}/Settings.h
        ${CORE_DIR}/System.h
        ${CORE_DIR}/Texture.h
        ${CORE_DIR}/Token.h
        ${CORE_DIR}/Type.h
        ${CORE_DIR}/Variant.h
        ${CORE_DIR}/Visibility.h
        ${CORE_DIR}/Way.h
        ${CORE_DIR}/Wire.h
        ${LANGUAGE_DIR}/Common/Function.h
        ${LANGUAGE_DIR}/Common/Language.h
        ${LANGUAGE_DIR}/Common/LanguageFactory.h
        ${LANGUAGE_DIR}/Common/Language_MACROS.h
        ${LANGUAGE_DIR}/Common/Operator.h
        ${LANGUAGE_DIR}/Common/Parser.h
        ${LANGUAGE_DIR}/Common/Semantic.h
        ${LANGUAGE_DIR}/Common/Serializer.h
        ${LANGUAGE_DIR}/Common/TokenRibbon.h
        ${LANGUAGE_DIR}/Common/TokenType.h
        ${LANGUAGE_DIR}/Nodable/NodableLanguage.h
        ${LANGUAGE_DIR}/Nodable/NodableParser.h
        ${LANGUAGE_DIR}/Nodable/NodableSerializer.h
        ${NODE_DIR}/AbstractCodeBlockNode.h
        ${NODE_DIR}/AbstractNodeFactory.h
        ${NODE_DIR}/CodeBlockNode.h
        ${NODE_DIR}/ConditionalStructNode.h
        ${NODE_DIR}/DefaultNodeFactory.h
        ${NODE_DIR}/GraphNode.h
        ${NODE_DIR}/GraphTraversal.h
        ${NODE_DIR}/InstructionNode.h
        ${NODE_DIR}/LiteralNode.h
        ${NODE_DIR}/Node.h
        ${NODE_DIR}/ProgramNode.h
        ${NODE_DIR}/ScopedCodeBlockNode.h
        ${NODE_DIR}/VariableNode.h
        ${UTILS_DIR}/ImGuiEx.h
        ${UTILS_DIR}/Utils/Maths.h
        )

# 2.c - Declare include directories
#----------------------------------
set( NODABLE_INCLUDE_DIRS
        ${BASE_DIR}
        ${LANGUAGE_DIR}
        ${CORE_DIR}
        ${NODE_DIR}
        ${COMPONENT_DIR})

# 3 - Post-script:
#=================

if( NODABLE_ENABLE_CONFIG_LOGS )
    message("nodable-config.cmake report:\n"
            "\tNODABLE_SOURCES are ${NODABLE_SOURCES}\n"
            "\tNODABLE_HEADERS are ${NODABLE_HEADERS}\n"
            "\tNODABLE_INCLUDE_DIRS are ${NODABLE_INCLUDE_DIRS}")
    message( "nodable-config.cmake DONE.")
endif()


