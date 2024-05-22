#include "GraphUtil.h"
#include "IfNode.h"

using namespace ndbl;
using namespace tools;

REGISTER
{
    registration::push_class<IfNode>("IfNode")
        .extends<Node>()
        .extends<IConditional>();
}

IfNode::IfNode() {} // required to link static code above