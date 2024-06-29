#include "LiteralNode.h"
#include "Slot.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<LiteralNode>("LiteralNode").extends<Node>();
}

LiteralNode::LiteralNode(const type* _type)
: Node()
, m_type( _type )
, m_value_property(nullptr)
{
}

void LiteralNode::init()
{
    Node::init();

    m_value_property = props.add(m_type, VALUE_PROPERTY, PropertyFlag_VISIBLE);
    add_slot(SlotFlag::SlotFlag_OUTPUT, 1, m_value_property);
    add_slot(SlotFlag::SlotFlag_OUTPUT, 1, m_this_as_property);

    add_slot( SlotFlag_PREV, Slot::MAX_CAPACITY);
}

Property *LiteralNode::value()
{
    ASSERT(m_value_property != nullptr)
    return m_value_property;
}

const Property *LiteralNode::value() const
{
    ASSERT(m_value_property != nullptr)
    return m_value_property;
}

