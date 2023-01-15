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
    m_props.add(k_value_property_name, Visibility::Always, _type, Way_Out, flags);
}
