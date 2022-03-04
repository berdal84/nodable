#include <nodable/LiteralNode.h>

using namespace Nodable;

R_DEFINE_CLASS(LiteralNode)

LiteralNode::LiteralNode(const R::Type* _type) : Node()
{
    m_props.add(Node::VALUE_MEMBER_NAME, Visibility::Always, _type, Way_Out);
}
