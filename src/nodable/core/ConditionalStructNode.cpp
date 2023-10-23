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
    m_next_slot[0] = add_slot( SlotFlag_NEXT, 1, m_this_property_id );
    m_next_slot[1] = add_slot( SlotFlag_NEXT, 1, m_this_property_id );
    m_child_slot[0] = add_slot( SlotFlag_CHILD, 1 , m_this_property_id );
    m_child_slot[1] = add_slot( SlotFlag_CHILD, 1 , m_this_property_id );
}

PoolID<Scope> ConditionalStructNode::get_scope_at(size_t _pos) const
{
    const Node* next = get_slot_at( m_next_slot.at(_pos) ).first_adjacent()->get_node();
    if ( next )
    {
        return next->get_component<Scope>();
    }
    return {};
}

bool ConditionalStructNode::is_chained_with_other_cond_struct() const
{
    if( auto false_scope = get_scope_at(Branch_FALSE ) )
    {
        return false_scope->get_owner()->get_type()->is_child_of<ConditionalStructNode>();
    }
    return false;
}

Slot& ConditionalStructNode::get_child_slot_at( size_t _pos )
{
    return get_slot_at( m_child_slot.at(_pos) );
}


const Slot& ConditionalStructNode::get_child_slot_at( size_t _pos ) const
{
    return get_slot_at( m_child_slot.at(_pos) );
}