#pragma once

#include "tools/core/unity_build.cpp"
#include "language/Nodlang.cpp"
#include "ASTFunctionCall.cpp"
#include "ASTLiteral.cpp"
#include "ASTNode.cpp"
#include "ASTNodeProperty.cpp"
#include "ASTNodeSlot.cpp"
#include "ASTScope.cpp"
#include "ASTSlotLink.cpp"
#include "ASTToken.cpp"
#include "ASTTokenRibbon.cpp"
#include "ASTVariable.cpp"
#include "ASTUtils.cpp"
#include "Graph.cpp"
#include "NodableHeadless.cpp"

namespace ndbl
{
    static void init_reflection()
    {
        tools::init_reflection();
        
        DEFINE_REFLECT(ASTFunctionCall).extends<ASTNode>();
        DEFINE_REFLECT(ASTLiteral).extends<ASTNode>();
        DEFINE_REFLECT(ASTNode);
        DEFINE_REFLECT(ASTScope).extends<Component<ASTNode>>();
        DEFINE_REFLECT(ASTVariable).extends<ASTNode>();
    } 
}