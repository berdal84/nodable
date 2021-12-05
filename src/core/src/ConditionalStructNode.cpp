#include <nodable/ConditionalStructNode.h>
#include <nodable/ScopedCodeBlockNode.h>

using namespace Nodable;

REFLECT_DEFINE(ConditionalStructNode)

ConditionalStructNode::ConditionalStructNode()
    :
    m_token_if(nullptr),
    m_token_else(nullptr)
{
    m_props.add("condition", Visibility::Always, Type_Boolean, Way::Way_In);
    setNextMaxCount(2); // allow 2 Nodes to be next.
}

ScopedCodeBlockNode *ConditionalStructNode::get_true_branch() const
{
    return !m_next.empty() ? m_next[0]->as<ScopedCodeBlockNode>() : nullptr;
}

ScopedCodeBlockNode *ConditionalStructNode::get_false_branch() const
{
    return m_next.size() > 1 ? m_next[1]->as<ScopedCodeBlockNode>() : nullptr;
}

bool ConditionalStructNode::has_instructions() const
{
    return (get_true_branch() && get_true_branch()->has_instructions())
            ||
            (get_false_branch() && get_false_branch()->has_instructions());
}

InstructionNode* ConditionalStructNode::get_first_instruction() const
{
    InstructionNode* first_instruction = nullptr;

    // TODO: not sure what to do here, should I branch here ?
    //       to keep code workig as-is, I choose not to check condition.
    if (get_true_branch() )
    {
        first_instruction = get_true_branch()->get_first_instruction();
    }

    if ( first_instruction == nullptr && get_false_branch())
    {
        first_instruction = get_false_branch()->get_first_instruction();
    }

    return  first_instruction;
}

void ConditionalStructNode::get_last_instructions(std::vector<InstructionNode *> &out)
{
    if (get_true_branch() )
    {
        get_true_branch()->get_last_instructions(out);
    }
    if (get_false_branch() )
    {
        get_false_branch()->get_last_instructions(out);
    }
}