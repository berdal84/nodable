#include "InstructionNode.h"

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<InstructionNode>("InstructionNode").extends<Node>();
}

void InstructionNode::init()
{
    Node::init();
    auto root_property_id = add_prop<PoolID<Node>>(ROOT_PROPERTY, PropertyFlag_VISIBLE);
    m_root_slot_id = add_slot( root_property_id, SlotFlag::SlotFlag_INPUT, 1 );

    find_slot( SlotFlag_PREV )->set_capacity( SLOT_MAX_CAPACITY );
    find_slot( SlotFlag_NEXT )->set_capacity( 1 );
}

Slot& InstructionNode::root_slot()
{
    return slots[m_root_slot_id];
}

const Slot& InstructionNode::root_slot() const
{
    return slots[m_root_slot_id];
}
