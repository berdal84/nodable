#include "WhileLoopNode.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    type::Initializer<WhileLoopNode>("WhileLoopNode").extends<Node>();
}

void WhileLoopNode::init(const std::string &_name)
{
    Node::init(NodeType_BLOCK_WHILE_LOOP, _name);
    m_wrapped_conditional.init(this);
}
