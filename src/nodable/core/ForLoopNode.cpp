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

    add_slot(init_id, SlotFlag::SlotFlag_INPUT, 1);
    add_slot(cond_id, SlotFlag::SlotFlag_INPUT, 1);
    add_slot(iter_id, SlotFlag::SlotFlag_INPUT, 1);

    set_slot_capacity( SlotFlag_PREV, SLOT_MAX_CAPACITY );
    set_slot_capacity( SlotFlag_NEXT, 1 );
}

PoolID<Scope> ForLoopNode::get_condition_true_scope() const
{
    return GraphUtil::adjacent_component_at<Scope>(this, SlotFlag::SlotFlag_NEXT, 0);
}

PoolID<Scope> ForLoopNode::get_condition_false_scope() const
{
    return GraphUtil::adjacent_component_at<Scope>(this, SlotFlag::SlotFlag_NEXT, 1);
}