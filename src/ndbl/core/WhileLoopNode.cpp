#include "WhileLoopNode.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<WhileLoopNode>("WhileLoopNode")
        .extends<Node>()
        .extends<IConditional>();
}

WhileLoopNode::WhileLoopNode() {} // required to link static code above