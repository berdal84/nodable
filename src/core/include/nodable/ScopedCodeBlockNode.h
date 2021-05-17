#pragma once
#include <nodable/AbstractCodeBlockNode.h> // abstract base class

namespace Nodable::core
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
    class ScopedCodeBlockNode: public AbstractCodeBlockNode
    {
    public:
        explicit ScopedCodeBlockNode();
        ~ScopedCodeBlockNode() override = default;

                      void                    clear() override;
        [[nodiscard]] bool                    isEmpty();
        [[nodiscard]] bool                    hasInstructions() const override;
        [[nodiscard]] InstructionNode*        getFirstInstruction() const override;
        [[nodiscard]] VariableNode*           findVariable(const std::string& _name) override;
        [[nodiscard]] AbstractCodeBlockNode*  getLastCodeBlock();
        [[nodiscard]] InstructionNode*        getLastInstruction();
        [[nodiscard]] inline const Token*     getBeginScopeToken() const { return m_beginScopeToken; }
        [[nodiscard]] inline const Token*     getEndScopeToken() const { return m_endScopeToken; }
        [[nodiscard]] inline const std::vector<VariableNode*>& getVariables()const { return m_variables; }

        void        addVariable(VariableNode*);
        inline void setBeginScopeToken(Token* token) { m_beginScopeToken = token; }
        inline void setEndScopeToken(Token* token) { m_endScopeToken = token; }

    private:
        Token* m_beginScopeToken;
        Token* m_endScopeToken;
        std::vector<VariableNode*> m_variables;

        /** Reflect class */
        MIRROR_CLASS(ScopedCodeBlockNode)
        (
            MIRROR_PARENT(AbstractCodeBlockNode)
        )
    };
}
