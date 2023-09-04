#include "LiteralNode.h"

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<LiteralNode>("LiteralNode").extends<Node>();
}

LiteralNode::LiteralNode(const fw::type* _type) : Node()
{
    Property::Flags flags = Property::Flags_initialize
                        | Property::Flags_define
                        | Property::Flags_reset_value;
    m_value_property_id = props.add(_type, VALUE_PROPERTY, Visibility::Always, Way::Out, flags);
}

LiteralNode::LiteralNode(): Node() {}
