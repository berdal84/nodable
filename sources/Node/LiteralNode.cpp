#include "LiteralNode.h"

Nodable::LiteralNode::LiteralNode(Nodable::Type type) : Node()
{
    m_props.add("value", Visibility::Always, type, Way_Out);
}
