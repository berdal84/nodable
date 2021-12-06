#include <nodable/ForLoopNode.h>
#include <nodable/ScopedCodeBlockNode.h>

using namespace Nodable;

REFLECT_DEFINE(ForLoopNode)

ForLoopNode::ForLoopNode()
        :
        m_token_for(nullptr)
{
    m_props.add("init"     , Visibility::Always, Type_Any     , Way::Way_In);
    m_props.add("condition", Visibility::Always, Type_Boolean , Way::Way_In);
    m_props.add("iter"     , Visibility::Always, Type_Any     , Way::Way_In);
    setPrevMaxCount(1); // allow 1 Nodes to be previous.
    setNextMaxCount(2); // allow 2 Nodes to be next (branches if and else).
}

ScopedCodeBlockNode *ForLoopNode::get_condition_true_branch() const
{
    return !m_next.empty() ? m_next[0]->as<ScopedCodeBlockNode>() : nullptr;
}

ScopedCodeBlockNode *ForLoopNode::get_condition_false_branch() const
{
    return m_next.size() > 1 ? m_next[1]->as<ScopedCodeBlockNode>() : nullptr;
}

bool ForLoopNode::has_instructions() const
{
    return (get_condition_true_branch() && get_condition_true_branch()->has_instructions())
           ||
           (get_condition_false_branch() && get_condition_false_branch()->has_instructions());
}

InstructionNode* ForLoopNode::get_first_instruction() const
{
    InstructionNode* first_instruction = nullptr;

    // TODO: not sure what to do here, should I branch here ?
    //       to keep code workig as-is, I choose not to check condition.
    if (get_condition_true_branch() )
    {
        first_instruction = get_condition_true_branch()->get_first_instruction();
    }

    if ( first_instruction == nullptr && get_condition_false_branch())
    {
        first_instruction = get_condition_false_branch()->get_first_instruction();
    }

    return  first_instruction;
}

void ForLoopNode::get_last_instructions(std::vector<InstructionNode *> &out)
{
    if (get_condition_true_branch() )
    {
        get_condition_true_branch()->get_last_instructions(out);
    }
    if (get_condition_false_branch() )
    {
        get_condition_false_branch()->get_last_instructions(out);
    }
}