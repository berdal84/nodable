#include "InstructionNode.h"

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<InstructionNode>("InstructionNode").extends<Node>();
}

InstructionNode::InstructionNode()
    : Node()
{
    add_prop<ID<Node>>(VALUE_PROPERTY, Visibility::Default, Way::In);
    slots.set_limit(NEXT_PREVIOUS, Way::In, EDGE_PER_SLOT_MAX_COUNT);
    slots.set_limit(NEXT_PREVIOUS, Way::Out, 1);
}
