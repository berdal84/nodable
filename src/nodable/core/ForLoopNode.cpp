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

    m_next_slot[0] = add_slot( SlotFlag_NEXT, 1 );
    m_next_slot[1] = add_slot( SlotFlag_NEXT, 1 );

    m_child_slot[0] = add_slot( SlotFlag_CHILD, 1 , m_this_property_id );
    m_child_slot[1] = add_slot( SlotFlag_CHILD, 1 , m_this_property_id );

    add_slot( SlotFlag_PARENT, 1);
}

PoolID<Scope> ForLoopNode::get_scope_at(size_t _pos) const
{
    const Node* next = get_slot_at( m_next_slot.at(_pos) ).first_adjacent()->get_node();
    if ( next )
    {
        return next->get_component<Scope>();
    }
    return {};
}

Slot& ForLoopNode::get_child_slot_at( size_t _pos )
{
    return get_slot_at( m_child_slot.at(_pos) );
}


const Slot& ForLoopNode::get_child_slot_at( size_t _pos ) const
{
    return get_slot_at( m_child_slot.at(_pos) );
}