#include "ForLoopNode.h"
#include "Node.h"
#include "GraphUtil.h"
#include "core/InstructionNode.h"
#include "core/Scope.h"

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<ForLoopNode>("ForLoopNode")
        .extends<Node>()
        .extends<IConditionalStruct>();
}

ForLoopNode::ForLoopNode()
{
    add_prop<PoolID<Node>>(INITIALIZATION_PROPERTY, Visibility::Always, Way::In); // for ( <here> ;   ..    ;   ..   ) { ... }
    add_prop<PoolID<Node>>(CONDITION_PROPERTY,      Visibility::Always, Way::In); // for (   ..   ; <here>  ;   ..   ) { ... }
    add_prop<PoolID<Node>>(ITERATION_PROPERTY,      Visibility::Always, Way::In); // for (   ..   ;   ..    ; <here> ) { ... }
}

PoolID<Scope> ForLoopNode::get_condition_true_scope() const
{
    return GraphUtil::adjacent_component_at<Scope>(this, Relation::NEXT_PREVIOUS, Way::Out, 0);
}

PoolID<Scope> ForLoopNode::get_condition_false_scope() const
{
    return GraphUtil::adjacent_component_at<Scope>(this, Relation::NEXT_PREVIOUS, Way::Out, 1);
}