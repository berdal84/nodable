#include <nodable/LiteralNode.h>

using namespace Nodable;

REFLECT_DEFINE(LiteralNode)

LiteralNode::LiteralNode(Type type) : Node()
{
    m_props.add("value", Visibility::Always, type, Way_Out);
}
