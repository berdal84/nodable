#pragma once

#include "tools/gui/unity_build.cpp"
#include "ndbl/core/unity_build.cpp"
#include "ASTNodePropertyView.cpp"
#include "ASTNodeSlotView.cpp"
#include "ASTNodeView.cpp"
#include "ASTNodeViewContextualMenu.cpp"
#include "ASTScopeView.cpp"
#include "Config.cpp"
#include "File.cpp"
#include "FileView.cpp"
#include "GraphView.cpp"
#include "History.cpp"
#include "Nodable.cpp"
#include "NodableView.cpp"
#include "PhysicsComponent.cpp"

namespace ndbl
{
    static void init_reflection_with_gui()
    {
        ndbl::init_reflection();
        
        DEFINE_REFLECT(ASTNodeView).extends<Component<ASTNode>>();
        DEFINE_REFLECT(GraphView).extends<tools::Component<Graph>>();
        DEFINE_REFLECT(PhysicsComponent).extends<Component<ASTNode>>();
    } 
}