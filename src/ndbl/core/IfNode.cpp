#include "GraphUtil.h"
#include "IfNode.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<IfNode>("IfNode").extends<Node>();
}


void IfNode::init(const std::string&_name)
{
    Node::init(NodeType_BLOCK_CONDITION, _name);
    m_wrapped_conditional.init(this);
}
// required to link static code above