#include <nodable/core/InstructionNode.h>
#include <nodable/core/Member.h>
#include <nodable/core/Log.h>

using namespace Nodable;
using namespace Nodable::R;

R_DEFINE_CLASS(InstructionNode)

InstructionNode::InstructionNode()
    : Node()
{
    m_props.add<Node*>(k_value_member_name, Visibility::Default, Way_In);
    m_successors.set_limit(1);
    m_predecessors.set_limit(-1);
}