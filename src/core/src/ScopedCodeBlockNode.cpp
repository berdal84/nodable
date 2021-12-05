#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/InstructionNode.h>
#include <nodable/VariableNode.h>
#include <nodable/CodeBlockNode.h>
#include <nodable/AbstractCodeBlock.h>
#include <cstring>
#include <algorithm> // for std::find_if
#include <nodable/Log.h>

using namespace Nodable;

REFLECT_DEFINE(CodeBlockNode)

ScopedCodeBlockNode::ScopedCodeBlockNode()
        :
        Node(),
        m_begin_scope_token(nullptr),
        m_end_scope_token(nullptr)
{
    setLabel("unnamed ScopedCodeBlockNode");
    setNextMaxCount(1);
    setPrevMaxCount(-1);
}

void ScopedCodeBlockNode::clear()
{
    m_variables.clear();
}

bool ScopedCodeBlockNode::has_instructions() const
{
    return get_first_instruction() != nullptr;
}

InstructionNode *ScopedCodeBlockNode::get_first_instruction() const
{
    auto found = std::find_if(
            m_children.begin(),
            m_children.end(),
            [](Node* block )
            {
                return block->as<CodeBlockNode>()->has_instructions();
            });

    if (found != m_children.end())
    {
        return (*found)->as<CodeBlockNode>()->get_first_instruction();
    }
    return nullptr;
}

VariableNode* ScopedCodeBlockNode::find_variable(const std::string &_name)
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

CodeBlockNode *ScopedCodeBlockNode::get_last_code_block()
{
    if ( m_children.empty() )
        return nullptr;

    auto back = m_children.back();
    if ( back )
    {
        return back->as<CodeBlockNode>();
    }
    return nullptr;
}

InstructionNode *ScopedCodeBlockNode::get_last_instruction()
{
    CodeBlockNode* lastCodeBlock = get_last_code_block();

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

void ScopedCodeBlockNode::add_variable(VariableNode* _variableNode)
{
    if (this->find_variable(_variableNode->getName()) == nullptr)
    {
        this->m_variables.push_back(_variableNode);
    }
    else
    {
        LOG_ERROR("ScopedCodeBlockNode", "Unable to add variable %s, already declared.\n", _variableNode->getName())
    }
}

void ScopedCodeBlockNode::get_last_instructions(std::vector<InstructionNode *> &out)
{
    CodeBlockNode::get_last_instructions(this, out);
}

Node* ScopedCodeBlockNode::get_parent() const
{
    return this->m_parent;
}
