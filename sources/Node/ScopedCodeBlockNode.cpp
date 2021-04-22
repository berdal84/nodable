#include "ScopedCodeBlockNode.h"
#include "Node/InstructionNode.h"
#include "Node/VariableNode.h"
#include "Node/CodeBlockNode.h"
#include "Node/AbstractCodeBlockNode.h"
#include <cstring>
#include <algorithm> // for std::find_if
#include <Log.h>

using namespace Nodable;

ScopedCodeBlockNode::ScopedCodeBlockNode()
        :
        AbstractCodeBlockNode(),
        beginScopeToken(nullptr),
        endScopeToken(nullptr)
{

}

ScopedCodeBlockNode::~ScopedCodeBlockNode(){}

void ScopedCodeBlockNode::clear()
{
    variables.clear();
}

bool ScopedCodeBlockNode::hasInstructions() const
{
    return getFirstInstruction() != nullptr;
}

InstructionNode *ScopedCodeBlockNode::getFirstInstruction() const
{
    auto found = std::find_if(
            children.begin(),
            children.end(),
            [](Node* block )
            {
                return block->as<AbstractCodeBlockNode>()->hasInstructions();
            });

    if ( found != children.end())
    {
        return (*found)->as<AbstractCodeBlockNode>()->getFirstInstruction();
    }
    return nullptr;
}

VariableNode* ScopedCodeBlockNode::findVariable(std::string _name)
{
    VariableNode* result = nullptr;

    auto findFunction = [_name](const VariableNode* _variable ) -> bool
    {
        return strcmp(_variable->getName(), _name.c_str()) == 0;
    };

    auto it = std::find_if(variables.begin(), variables.end(), findFunction);
    if (it != variables.end()){
        result = *it;
    }

    return result;
}

AbstractCodeBlockNode *ScopedCodeBlockNode::getLastCodeBlock()
{
    if ( children.empty() )
        return nullptr;

    auto back = children.back();
    if ( back )
    {
        return back->as<AbstractCodeBlockNode>();
    }
    return nullptr;
}

bool ScopedCodeBlockNode::isEmpty()
{
    return children.empty();
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
        this->variables.push_back(_variableNode);
    else
        LOG_ERROR("ScopedCodeBlockNode", "Unable to add variable %s, already declared.\n", _variableNode->getName());
}


