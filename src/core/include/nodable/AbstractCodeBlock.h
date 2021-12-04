#pragma once
#include <vector>

namespace Nodable
{

    // forward declarations
    class InstructionNode;
    class VariableNode;
    class Node;

    /**
     * Interface for ScopedCodeBlockNode and CodeBlockNode
     */
    class AbstractCodeBlock
    {
    public:
        virtual void                   clear() = 0;
        virtual bool                   has_instructions() const = 0;
        virtual InstructionNode*       get_first_instruction() const = 0;
        virtual void                   get_last_instructions(std::vector<InstructionNode *> &out) = 0;
        virtual VariableNode*          find_variable(const std::string&) = 0;
        virtual Node*                  get_parent()const = 0;
    };
}
