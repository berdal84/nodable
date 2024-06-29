#include "GraphUtil.h"
#include "IfNode.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<IfNode>("IfNode")
        .extends<Node>()
        .extends<IConditional>();
}

IfNode::IfNode() {}

void IfNode::init(const std::string&_name)
{
    TConditionalNode::init(NodeType_BLOCK_CONDITION, _name);
}
// required to link static code above