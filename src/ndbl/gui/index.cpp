#include "index.h"
#include "tools/core/Component.h"
#include "tools/core/reflection/Initializer.h"
#include "ndbl/core/index.h"
#include "ndbl/core/ASTNode.h"
#include "ndbl/gui/GraphView.h"
#include "ndbl/gui/PhysicsComponent.h"

void ndbl::init_reflection_with_gui()
{
    ndbl::init_reflection();
    
    DEFINE_REFLECT(ndbl::ASTNodeView).extends<tools::Component<ASTNode>>();
    DEFINE_REFLECT(ndbl::GraphView).extends<tools::Component<Graph>>();
    DEFINE_REFLECT(ndbl::PhysicsComponent).extends<tools::Component<ASTNode>>();
} 