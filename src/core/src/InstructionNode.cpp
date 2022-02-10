#include <nodable/InstructionNode.h>
#include <nodable/Member.h>
#include <nodable/Log.h>

using namespace Nodable;
using namespace Nodable::Reflect;

REFLECT_DEFINE_CLASS(InstructionNode)

InstructionNode::InstructionNode(const char* _label)
    : Node(_label)
{
    /*
     * Add a member to identify the (root) node to evaluate when this instruction is processed by Asm::Compiler.
     */
    m_props.add("root_node", Visibility::Default, Type_Pointer, Way_In);
    m_successors.set_max_count(1);
    m_predecessors.set_max_count(-1);
}