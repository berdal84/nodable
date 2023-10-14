#include "ConditionalStructNode.h"
#include "Scope.h"
#include "InstructionNode.h"
#include "GraphUtil.h"

using namespace ndbl;

REGISTER
{
    using namespace ndbl;
    fw::registration::push_class<ConditionalStructNode>("ConditionalStructNode")
        .extends<Node>()
        .extends<IConditionalStruct>();
}

void ConditionalStructNode::init()
{
    Node::init();

    auto cond_id = add_prop<PoolID<Node>>(CONDITION_PROPERTY, PropertyFlag_VISIBLE);
    add_slot( SlotFlag::SlotFlag_INPUT, 1, cond_id );
    add_slot( SlotFlag_PREV, SLOT_MAX_CAPACITY );
    add_slot( SlotFlag_PARENT, 1);
    add_slot( SlotFlag_OUTPUT, SLOT_MAX_CAPACITY );
    add_slot( SlotFlag_NEXT, 2 );
    add_slot( SlotFlag_CHILD, 2 );
}

PoolID<Scope> ConditionalStructNode::get_condition_true_scope() const
{
    return GraphUtil::adjacent_component_at<Scope>(this, SlotFlag_NEXT, 0);
}

PoolID<Scope> ConditionalStructNode::get_condition_false_scope() const
{
    return GraphUtil::adjacent_component_at<Scope>(this, SlotFlag_NEXT, 1);
}

bool ConditionalStructNode::is_chained_with_other_cond_struct() const
{
    if( auto false_scope = get_condition_false_scope() )
    {
        return false_scope->get_owner()->get_type()->is_child_of<ConditionalStructNode>();
    }
    return false;
}

const Property* ConditionalStructNode::condition_property() const
{
    return get_prop(CONDITION_PROPERTY);
}