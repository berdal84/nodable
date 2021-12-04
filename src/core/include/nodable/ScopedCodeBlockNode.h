#pragma once
#include <string>

#include <nodable/Node.h> // base class
#include <nodable/AbstractCodeBlock.h> // interface

namespace Nodable
{
    // Forward declarations
    class InstructionNode;
    class CodeBlockNode;

    /**
     * A Scoped code block contains:
     * - AbstractCodeBlocks (CodeBlock or ScopedCodeBlock)
     * - VariableNodes
     * All of them are NOT owned by this class.
     */
    class ScopedCodeBlockNode: public Node, public AbstractCodeBlock
    {
    public:
        explicit ScopedCodeBlockNode();
        ~ScopedCodeBlockNode() override = default;

        void                    clear() override;
        bool                    has_instructions() const override;
        InstructionNode*        get_first_instruction() const override;
        CodeBlockNode*          get_last_code_block();
        InstructionNode*        get_last_instruction();
        void                    get_last_instructions(std::vector<InstructionNode *> &out) override ;
        inline const Token*     get_begin_scope_token() const { return m_begin_scope_token; }
        inline const Token*     get_end_scope_token() const { return m_end_scope_token; }
        inline void             set_begin_scope_token(Token *token) { m_begin_scope_token = token; }
        inline void             set_end_Scope_token(Token *token) { m_end_scope_token = token; }
        void                    add_variable(VariableNode*);
        VariableNode*           find_variable(const std::string &_name) override;
        Node*                   get_parent() const override ;
        inline const std::vector<VariableNode*>& get_variables()const { return m_variables; }

    private:
        Token* m_begin_scope_token;
        Token* m_end_scope_token;
        std::vector<VariableNode*> m_variables;

        /** Reflect class */
        REFLECT_DERIVED(ScopedCodeBlockNode)
          REFLECT_EXTENDS(Node)
        REFLECT_END
    };
}
