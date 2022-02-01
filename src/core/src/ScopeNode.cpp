#include <nodable/ScopeNode.h>

#include <cstring>
#include <algorithm> // for std::find_if
#include <nodable/Log.h>
#include <nodable/ConditionalStructNode.h>
#include <nodable/ForLoopNode.h>
#include <nodable/InstructionNode.h>
#include <nodable/VariableNode.h>
#include <nodable/AbstractCodeBlock.h>

using namespace Nodable;

REFLECT_DEFINE(ScopeNode)

ScopeNode::ScopeNode()
        :
        Node(),
        m_begin_scope_token(nullptr),
        m_end_scope_token(nullptr)
{
    setLabel("unnamed ScopeNode");
    setNextMaxCount(1);
    setPrevMaxCount(-1);
}

void ScopeNode::clear()
{
    m_variables.clear();
}

VariableNode* ScopeNode::find_variable(const std::string &_name)
{
    VariableNode* result = nullptr;

    auto findFunction = [_name](const VariableNode* _variable ) -> bool
    {
        return strcmp(_variable->getName(), _name.c_str()) == 0;
    };

    auto it = std::find_if(m_variables.begin(), m_variables.end(), findFunction);
    if (it != m_variables.end()){
        result = *it;
    }

    return result;
}

ScopeNode* ScopeNode::get_last_code_block()
{
    if ( m_children.empty() )
        return nullptr;

    auto back = m_children.back();
    if ( back )
    {
        return back->as<ScopeNode>();
    }
    return nullptr;
}

InstructionNode *ScopeNode::get_last_instruction()
{
    ScopeNode* lastCodeBlock = get_last_code_block();

    if ( lastCodeBlock )
    {
        auto instructions = lastCodeBlock->get_children();
        if ( !instructions.empty())
        {
            Node* instr = instructions.back();
            NODABLE_ASSERT( instr->get_class()->is<InstructionNode>() );
            return instr->as<InstructionNode>();
        }
    }

    return nullptr;
}

void ScopeNode::add_variable(VariableNode* _variableNode)
{
    if (this->find_variable(_variableNode->getName()) == nullptr)
    {
        this->m_variables.push_back(_variableNode);
    }
    else
    {
        LOG_ERROR("ScopeNode", "Unable to add variable %s, already declared.\n", _variableNode->getName())
    }
}

void ScopeNode::get_last_instructions(std::vector<InstructionNode *> &out)
{
    ScopeNode::get_last_instructions(this, out);
}

Node* ScopeNode::get_parent() const
{
    return this->m_parent;
}

void ScopeNode::get_last_instructions(Node* _node, std::vector<InstructionNode *> & _out)
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
        else if (auto* node = last->as<ScopeNode>())
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
