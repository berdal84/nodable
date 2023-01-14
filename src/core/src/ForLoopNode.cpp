#include <nodable/core/ForLoopNode.h>
#include <nodable/core/Scope.h>

using namespace ndbl;

REGISTER
{
    registration::push_class<ForLoopNode>("ForLoopNode")
        .extends<Node>()
        .extends<IConditionalStruct>();
}

ForLoopNode::ForLoopNode()
    : m_token_for(nullptr)
    , m_init_instr_node(nullptr)
    , m_cond_instr_node(nullptr)
    , m_iter_instr_node(nullptr)
{
    m_props.add<Node*>(k_forloop_initialization_member_name , Visibility::Always, Way::Way_In);
    m_props.add<Node*>(k_condition_member_name              , Visibility::Always, Way::Way_In);
    m_props.add<Node*>(k_forloop_iteration_member_name      , Visibility::Always, Way::Way_In);
}

Scope* ForLoopNode::get_condition_true_scope() const
{
    return !m_successors.empty() ? m_successors[0]->get<Scope>() : nullptr;
}

Scope*  ForLoopNode::get_condition_false_scope() const
{
    return m_successors.size() > 1 ? m_successors[1]->get<Scope>() : nullptr;
}

void ForLoopNode::set_iter_instr(InstructionNode* _node)
{
    m_iter_instr_node = _node;
}

void ForLoopNode::set_init_instr(InstructionNode* _node)
{
    m_init_instr_node = _node;
}

void ForLoopNode::set_cond_expr(InstructionNode* _node)
{
    m_cond_instr_node = _node;
}