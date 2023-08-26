#include "InstructionNode.h"

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<InstructionNode>("InstructionNode").extends<Node>();
}

InstructionNode::InstructionNode()
    : Node()
{
    m_root = add_prop<ID<Node>>(k_value_property_name, Visibility::Default, Way_In);
    successors.set_limit(1);
    predecessors.set_limit(-1);
}
