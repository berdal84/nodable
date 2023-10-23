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
    add_slot( SlotFlag_INPUT, 1, cond_id );

    add_slot( SlotFlag_PREV, SLOT_MAX_CAPACITY, m_this_property_id );
    add_slot( SlotFlag_PARENT, 1, m_this_property_id );

    m_next_slot[0] = add_slot( SlotFlag_NEXT, 1, m_this_property_id );
    m_next_slot[1] = add_slot( SlotFlag_NEXT, 1, m_this_property_id );

    m_child_slot[0] = add_slot( SlotFlag_CHILD, 1 , m_this_property_id );
    m_child_slot[1] = add_slot( SlotFlag_CHILD, 1 , m_this_property_id );
}

PoolID<Scope> WhileLoopNode::get_scope_at(size_t _pos) const
{
    const Node* next = get_slot_at( m_next_slot.at(_pos) ).first_adjacent()->get_node();
    if ( next )
    {
        return next->get_component<Scope>();
    }
    return {};
}


Slot& WhileLoopNode::get_child_slot_at( size_t _pos )
{
    return get_slot_at( m_child_slot.at(_pos) );
}


const Slot& WhileLoopNode::get_child_slot_at( size_t _pos ) const
{
    return get_slot_at( m_child_slot.at(_pos) );
}