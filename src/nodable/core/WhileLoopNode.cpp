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

void WhileLoopNode::init()
{
    Node::init();

    auto cond_id = add_prop<PoolID<Node>>( CONDITION_PROPERTY ); // while ( <here> ) { ... }
    add_slot( cond_id, SlotFlag_INPUT, 1 );

    set_limit(SlotFlag_PREV, SLOT_MAX_CAPACITY);
    set_limit(SlotFlag_NEXT, 1);
}

PoolID<Scope> WhileLoopNode::get_condition_true_scope() const
{
    return GraphUtil::adjacent_component_at<Scope>(this, SlotFlag_NEXT, 0);
}

PoolID<Scope> WhileLoopNode::get_condition_false_scope() const
{
    return GraphUtil::adjacent_component_at<Scope>(this, SlotFlag_NEXT, 1);
}


const Property* WhileLoopNode::condition_property() const
{
    return get_prop(CONDITION_PROPERTY);
}
