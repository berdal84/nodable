#include <nodable/core/InstructionNode.h>
#include <nodable/core/Log.h>
#include <nodable/core/Property.h>

using namespace ndbl;

REGISTER
{
    registration::push_class<InstructionNode>("InstructionNode").extends<Node>();
}

InstructionNode::InstructionNode()
    : Node()
{
    m_props.add<Node*>(k_value_property_name, Visibility::Default, Way_In);
    m_successors.set_limit(1);
    m_predecessors.set_limit(-1);
}