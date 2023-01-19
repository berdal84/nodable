#include <nodable/core/LiteralNode.h>

using namespace ndbl;

REGISTER
{
    registration::push_class<LiteralNode>("LiteralNode").extends<Node>();
}

LiteralNode::LiteralNode(type _type) : Node()
{
    Property::Flags flags = Property::Flags_initialize
                        | Property::Flags_define
                        | Property::Flags_reset_value;
    m_props.add(_type, k_value_property_name, Visibility::Always, Way_Out, flags);
}
