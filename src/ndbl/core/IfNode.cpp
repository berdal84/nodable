#include "Utils.h"
#include "IfNode.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    type::Initializer<IfNode>("IfNode").extends<Node>();
}


void IfNode::init(const std::string&_name)
{
    Node::init(NodeType_BLOCK_IF, _name);
    SwitchBehavior::init(this, 2);
}
// required to link static code above