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
    setPrevMaxCount(1); // allow 1 Nodes to be previous.
    setNextMaxCount(2); // allow 2 Nodes to be next (branches if and else).
}

ScopedCodeBlockNode *ConditionalStructNode::get_if_branch() const
{
    return !m_next.empty() ? m_next[0]->as<ScopedCodeBlockNode>() : nullptr;
}

ScopedCodeBlockNode *ConditionalStructNode::get_else_branch() const
{
    return m_next.size() > 1 ? m_next[1]->as<ScopedCodeBlockNode>() : nullptr;
}

bool ConditionalStructNode::has_instructions() const
{
    return (get_if_branch() && get_if_branch()->has_instructions())
            ||
            (get_else_branch() && get_else_branch()->has_instructions());
}

InstructionNode* ConditionalStructNode::get_first_instruction() const
{
    InstructionNode* first_instruction = nullptr;

    // TODO: not sure what to do here, should I branch here ?
    //       to keep code workig as-is, I choose not to check condition.
    if (get_if_branch() )
    {
        first_instruction = get_if_branch()->get_first_instruction();
    }

    if ( first_instruction == nullptr && get_else_branch())
    {
        first_instruction = get_else_branch()->get_first_instruction();
    }

    return  first_instruction;
}

void ConditionalStructNode::get_last_instructions(std::vector<InstructionNode *> &out)
{
    if (get_if_branch() )
    {
        get_if_branch()->get_last_instructions(out);
    }
    if (get_else_branch() )
    {
        get_else_branch()->get_last_instructions(out);
    }
}