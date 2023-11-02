#include "GraphUtil.h"
#include "IfNode.h"
#include "InstructionNode.h"

using namespace ndbl;

REGISTER
{
    using namespace ndbl;
    fw::registration::push_class<IfNode>("IfNode")
        .extends<Node>()
        .extends<IConditional>();
}

IfNode::IfNode() {} // required to link static code above