#include <nodable/core/InstructionNode.h>
#include <nodable/core/Member.h>
#include <nodable/core/Log.h>

using namespace Nodable;
using namespace Nodable::R;

R_DEFINE_CLASS(InstructionNode)

InstructionNode::InstructionNode(const char* _label)
    : Node(_label)
{
    /*
     * Add a member to identify the (root) node to evaluate when this instruction is processed by Asm::Compiler.
     */
    m_props.add("root_node", Visibility::Default, R::get_meta_type<Node *>(), Way_In);
    m_successors.set_limit(1);
    m_predecessors.set_limit(-1);
}