#include "WhileLoopNode.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INITIALIZER
(
    DEFINE_REFLECT(WhileLoopNode).extends<Node>();
)

void WhileLoopNode::init(const std::string &_name)
{
    Node::init(NodeType_BLOCK_WHILE_LOOP, _name);
    SwitchBehavior::init(this, 2);
}
