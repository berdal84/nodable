#include "LiteralNode.h"
#include "Slot.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<LiteralNode>("LiteralNode").extends<Node>();
}

void LiteralNode::init(const TypeDesc* _type, const std::string& _name)
{
    Node::init(NodeType_LITERAL, _name);
    m_type = _type;
    m_value_property = m_props.add(m_type, VALUE_PROPERTY);
    add_slot(SlotFlag::SlotFlag_OUTPUT, 1, m_value_property);
    add_slot(SlotFlag::SlotFlag_OUTPUT, 1, m_this_as_property);

    add_slot( SlotFlag_PREV, Slot::MAX_CAPACITY);
}

Slot& LiteralNode::output_slot()
{
    return const_cast<Slot&>( const_cast<const LiteralNode*>(this)->output_slot() );
}

const Slot& LiteralNode::output_slot() const
{
    return *find_slot_by_property(m_value_property, SlotFlag_OUTPUT );
}
