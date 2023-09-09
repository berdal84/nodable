#include "WhileLoopNode.h"
#include "core/Scope.h"
#include "InstructionNode.h"
#include "GraphUtil.h"

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<WhileLoopNode>("WhileLoopNode")
        .extends<Node>()
        .extends<IConditionalStruct>();
}

WhileLoopNode::WhileLoopNode()
{
    add_prop<ID<Node>>(CONDITION_PROPERTY, Visibility::Always, Way::In); // while ( <here> ) { ... }
}

PoolID<Scope> WhileLoopNode::get_condition_true_scope() const
{
    return GraphUtil::adjacent_component_at<Scope>(this, Relation::NEXT_PREVIOUS, Way::Out, 0);
}

PoolID<Scope> WhileLoopNode::get_condition_false_scope() const
{
    return GraphUtil::adjacent_component_at<Scope>(this, Relation::NEXT_PREVIOUS, Way::Out, 1);
}


const Property* WhileLoopNode::condition_property() const
{
    return get_prop(CONDITION_PROPERTY);
}
