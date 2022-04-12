#include <nodable/core/LiteralNode.h>

using namespace Nodable;

REGISTER
{
    registration::push_class<LiteralNode>("LiteralNode").extends<Node>();
}

LiteralNode::LiteralNode(type _type) : Node()
{
    m_props.add(k_value_member_name, Visibility::Always, _type, Way_Out);
}
