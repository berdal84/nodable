#include <nodable/core/LiteralNode.h>

using namespace Nodable;

R_DEFINE_CLASS(LiteralNode)

LiteralNode::LiteralNode(std::shared_ptr<const R::Meta_t> _type) : Node()
{
    m_props.add(k_value_member_name, Visibility::Always, _type, Way_Out);
}
