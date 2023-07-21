#include "InstructionNode.h"

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<InstructionNode>("InstructionNode").extends<Node>();
}

InstructionNode::InstructionNode()
    : Node()
{
    auto property = props.add<Node*>(k_value_property_name, Visibility::Default, Way_In);
    this->root = property.get();
    successors.set_limit(1);
    predecessors.set_limit(-1);
}