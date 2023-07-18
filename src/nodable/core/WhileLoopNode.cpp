#include "WhileLoopNode.h"
#include "core/Scope.h"

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<WhileLoopNode>("WhileLoopNode")
        .extends<Node>()
        .extends<IConditionalStruct>();
}

WhileLoopNode::WhileLoopNode()
    : m_cond_instr_node(nullptr)
{
    m_props.add<Node*>(k_conditional_cond_property_name, Visibility::Always, Way::Way_In); // for (   ..   ; <here>  ;   ..   ) { ... }
}

Scope* WhileLoopNode::get_condition_true_scope() const
{
    return !m_successors.empty() ? m_successors[0]->get_component<Scope>() : nullptr;
}

Scope*  WhileLoopNode::get_condition_false_scope() const
{
    return m_successors.size() > 1 ? m_successors[1]->get_component<Scope>() : nullptr;
}

void WhileLoopNode::set_cond_expr(InstructionNode* _node)
{
    m_cond_instr_node = _node;
}