#include "WhileLoopNode.h"

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<WhileLoopNode>("WhileLoopNode")
        .extends<Node>()
        .extends<IConditional>();
}

WhileLoopNode::WhileLoopNode() {} // required to link static code above