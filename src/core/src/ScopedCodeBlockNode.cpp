#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/InstructionNode.h>
#include <nodable/VariableNode.h>
#include <nodable/CodeBlockNode.h>
#include <nodable/AbstractCodeBlockNode.h>
#include <cstring>
#include <algorithm> // for std::find_if
#include <nodable/Log.h>

using namespace Nodable::core;

ScopedCodeBlockNode::ScopedCodeBlockNode()
        :
        AbstractCodeBlockNode(),
        m_beginScopeToken(nullptr),
        m_endScopeToken(nullptr)
{

}

void ScopedCodeBlockNode::clear()
{
    m_variables.clear();
}

bool ScopedCodeBlockNode::hasInstructions() const
{
    return getFirstInstruction() != nullptr;
}

InstructionNode *ScopedCodeBlockNode::getFirstInstruction() const
{
    auto found = std::find_if(
            m_children.begin(),
            m_children.end(),
            [](Node* block )
            {
                return block->as<AbstractCodeBlockNode>()->hasInstructions();
            });

    if (found != m_children.end())
    {
        return (*found)->as<AbstractCodeBlockNode>()->getFirstInstruction();
    }
    return nullptr;
}

VariableNode* ScopedCodeBlockNode::findVariable(const std::string& _name)
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

AbstractCodeBlockNode *ScopedCodeBlockNode::getLastCodeBlock()
{
    if ( m_children.empty() )
        return nullptr;

    auto back = m_children.back();
    if ( back )
    {
        return back->as<AbstractCodeBlockNode>();
    }
    return nullptr;
}

bool ScopedCodeBlockNode::isEmpty()
{
    return m_children.empty();
}

InstructionNode *ScopedCodeBlockNode::getLastInstruction()
{
    auto lastCodeBlock = getLastCodeBlock();

    if ( lastCodeBlock )
    {
        auto instructions = lastCodeBlock->as<CodeBlockNode>()->getChildren();
        if ( !instructions.empty())
        {
            Node* instr = instructions.back();
            NODABLE_ASSERT(instr->getClass() == InstructionNode::GetClass());
            return instr->as<InstructionNode>();
        }
    }

    return nullptr;
}

void ScopedCodeBlockNode::addVariable(VariableNode* _variableNode)
{
    if ( this->findVariable(_variableNode->getName()) == nullptr)
        this->m_variables.push_back(_variableNode);
    else
        LOG_ERROR("ScopedCodeBlockNode", "Unable to add variable %s, already declared.\n", _variableNode->getName());
}


