#pragma once
#include <nodable/AbstractCodeBlock.h>
#include <nodable/Node.h>

namespace Nodable
{
    // Forward declarations
    class InstructionNode;
    class VariableNode;

    /**
     * A Code block class to contain a set of instructions.
     * This class can't contain other Code blocks.
     */
    class CodeBlockNode: public Node, public AbstractCodeBlock
    {
    public:
        explicit CodeBlockNode();
        ~CodeBlockNode();

        void                    clear() override;
        bool                    has_instructions() const override ;
        InstructionNode*        get_first_instruction() const override ;
        void                    get_last_instructions(std::vector<InstructionNode *> &out) override ;
        static void             get_last_instructions(Node* _node, std::vector<InstructionNode *> & _out);
        VariableNode*           find_variable(const std::string &_name) override;
        Node*                   get_parent() const override ;
        REFLECT_DERIVED(CodeBlockNode)
          REFLECT_EXTENDS(Node)
        REFLECT_END
    };
}
