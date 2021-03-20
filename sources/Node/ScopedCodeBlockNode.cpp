#include "ScopedCodeBlockNode.h"
#include "Node/InstructionNode.h"
#include "Node/VariableNode.h"
#include "Node/CodeBlockNode.h"
#include "Node/AbstractCodeBlockNode.h"
#include <cstring>
#include <algorithm> // for std::find_if

using namespace Nodable;

ScopedCodeBlockNode::~ScopedCodeBlockNode()
{
    clear();
}

void ScopedCodeBlockNode::clear()
{
    getChildren().clear();
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
        auto instructions = lastCodeBlock->as<CodeBlockNode>()->getInstructions();
        if ( !instructions.empty())
        {
            return instructions.back()->as<InstructionNode>();
        }
    }

    return nullptr;
}