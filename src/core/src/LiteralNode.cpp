#include <nodable/LiteralNode.h>

using namespace Nodable;

R_DEFINE_CLASS(LiteralNode)

LiteralNode::LiteralNode(R::Type type) : Node()
{
    m_props.add(Node::VALUE_MEMBER_NAME, Visibility::Always, type, Way_Out);
}
