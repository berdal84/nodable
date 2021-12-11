#include <nodable/CodeBlockNode.h>
#include <nodable/InstructionNode.h>
#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/ConditionalStructNode.h>
#include <nodable/ForLoopNode.h>

using namespace Nodable;

REFLECT_DEFINE(CodeBlockNode)

CodeBlockNode::CodeBlockNode()
        :
        Node()
{
    setLabel("unnamed CodeBlockNode");
    setNextMaxCount(1);
    setPrevMaxCount(-1);
}

CodeBlockNode::~CodeBlockNode(){}

void CodeBlockNode::clear()
{
    // a code block do NOT owns its instructions nodes
    m_children.clear();
}

bool CodeBlockNode::has_instructions() const
{
    return !m_children.empty();
}

InstructionNode* CodeBlockNode::get_first_instruction() const
{
    return m_children.front()->as<InstructionNode>();
}

Node* CodeBlockNode::get_parent() const
{
    return this->m_parent;
}

void CodeBlockNode::get_last_instructions(std::vector<InstructionNode *> &out)
{
    CodeBlockNode::get_last_instructions(this, out);
}

void CodeBlockNode::get_last_instructions(Node* _node, std::vector<InstructionNode *> & _out)
{
    if (_node->get_children().empty())
        return;

    Node *last = _node->get_children().back();
    if (last)
    {
        if (auto* node = last->as<InstructionNode>())
        {
            _out.push_back(node);
        }
        else if (auto* node = last->as<CodeBlockNode>())
        {
            node->get_last_instructions(_out);
        }
        else if (auto* node = last->as<ConditionalStructNode>())
        {
            node->get_last_instructions(_out);
        }
        else if (auto* node = last->as<ForLoopNode>())
        {
            node->get_last_instructions(_out);
        }
    }
}