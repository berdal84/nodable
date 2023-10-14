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
    m_root_slot_id = add_slot( SlotFlag::SlotFlag_INPUT, 1, root_property_id);

    add_slot( SlotFlag_PREV, SLOT_MAX_CAPACITY, m_this_property_id );
    add_slot( SlotFlag_NEXT, 1, m_this_property_id );
    add_slot( SlotFlag_PARENT, 1, m_this_property_id);
    add_slot( SlotFlag_OUTPUT, 1, m_this_property_id);
}

Slot& InstructionNode::root_slot()
{
    return slots[(u8_t)m_root_slot_id];
}

const Slot& InstructionNode::root_slot() const
{
    return slots[(u8_t)m_root_slot_id];
}
