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

IfNode::IfNode() {} // required to link static code above