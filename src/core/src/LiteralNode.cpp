#include <nodable/LiteralNode.h>

using namespace Nodable;

REFLECT_CLASS_DEFINITION(LiteralNode)

LiteralNode::LiteralNode(Type type) : Node()
{
    m_props.add("value", Visibility::Always, type, Way_Out);
}
