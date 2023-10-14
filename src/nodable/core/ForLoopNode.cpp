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

void ForLoopNode::init()
{
    Node::init();

    auto init_id = add_prop<PoolID<Node>>(INITIALIZATION_PROPERTY, PropertyFlag_VISIBLE ); // for ( <here> ;   ..    ;   ..   ) { ... }
    auto cond_id = add_prop<PoolID<Node>>(CONDITION_PROPERTY,      PropertyFlag_VISIBLE ); // for (   ..   ; <here>  ;   ..   ) { ... }
    auto iter_id = add_prop<PoolID<Node>>(ITERATION_PROPERTY,      PropertyFlag_VISIBLE ); // for (   ..   ;   ..    ; <here> ) { ... }

    add_slot( SlotFlag_INPUT, 1, init_id);
    add_slot( SlotFlag_INPUT, 1, cond_id);
    add_slot( SlotFlag_INPUT, 1, iter_id);

    add_slot( SlotFlag_PREV, SLOT_MAX_CAPACITY );
    add_slot( SlotFlag_NEXT, 2 );
    add_slot( SlotFlag_CHILD, 2 );
    add_slot( SlotFlag_PARENT, 1);
}

PoolID<Scope> ForLoopNode::get_condition_true_scope() const
{
    return GraphUtil::adjacent_component_at<Scope>(this, SlotFlag_NEXT, 0);
}

PoolID<Scope> ForLoopNode::get_condition_false_scope() const
{
    return GraphUtil::adjacent_component_at<Scope>(this, SlotFlag_NEXT, 1);
}