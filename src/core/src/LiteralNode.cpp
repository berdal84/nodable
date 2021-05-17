#include <nodable/LiteralNode.h>

using namespace Nodable::core;

LiteralNode::LiteralNode(Type type) : Node()
{
    m_props.add("value", Visibility::Always, type, Way_Out);
}
