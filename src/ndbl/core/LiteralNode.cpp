#include "LiteralNode.h"

using namespace ndbl;
using namespace tools;

REGISTER
{
    registration::push_class<LiteralNode>("LiteralNode").extends<Node>();
}

LiteralNode::LiteralNode(const type* _type)
: Node()
, m_type( _type )
{
}

void LiteralNode::init()
{
    Node::init();

    m_value_property_id = props.add(
            m_type,
            VALUE_PROPERTY,
            PropertyFlag_VISIBLE | PropertyFlag_INITIALIZE | PropertyFlag_DEFINE | PropertyFlag_RESET_VALUE);
    add_slot( SlotFlag::SlotFlag_OUTPUT, 1, m_value_property_id);
    add_slot( SlotFlag::SlotFlag_OUTPUT, 1, m_this_property_id);

    add_slot( SlotFlag_PREV, SLOT_MAX_CAPACITY);
}

