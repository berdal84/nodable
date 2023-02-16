#include "fw/core/log.h"

#include <ndbl/core/InstructionNode.h>
#include <ndbl/core/Property.h>

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<InstructionNode>("InstructionNode").extends<Node>();
}

InstructionNode::InstructionNode()
    : Node()
{
    m_props.add<Node*>(k_value_property_name, Visibility::Default, Way_In);
    m_successors.set_limit(1);
    m_predecessors.set_limit(-1);
}