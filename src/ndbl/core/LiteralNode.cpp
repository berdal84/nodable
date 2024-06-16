#include "LiteralNode.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<LiteralNode>("LiteralNode").extends<Node>();
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
    add_slot(SlotFlag::SlotFlag_OUTPUT, 1, m_this_as_property);

    add_slot( SlotFlag_PREV, Slot::MAX_CAPACITY);
}

Property *LiteralNode::value()
{
    ASSERT(m_value_property_id != nullptr)
    return m_value_property_id;
}

const Property *LiteralNode::value() const
{
    ASSERT(m_value_property_id != nullptr)
    return m_value_property_id;
}

