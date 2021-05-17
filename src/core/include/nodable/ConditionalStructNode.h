#pragma once

#include <nodable/CodeBlockNode.h> // base class

namespace Nodable::core
{
    // forward declarations
    class ScopedCodeBlockNode;

    /**
     * @brief Class to represent a conditional structure ( IF/ELSE )
     */
    class ConditionalStructNode: public CodeBlockNode {
    public:
        ConditionalStructNode();
        ~ConditionalStructNode() = default;

        inline void setCondition(Member* _value) const { getCondition()->set(_value); }
        inline void setTokenIf(Token* token) { m_token_if = token; }
        inline void setTokenElse(Token* token) { m_token_else = token; }

        [[nodiscard]] ScopedCodeBlockNode*   getBranchTrue()const;
        [[nodiscard]] ScopedCodeBlockNode*   getBranchFalse()const;
        [[nodiscard]] inline Member*         getCondition()const { return m_props.get("condition"); }
        [[nodiscard]] inline const Token*    getTokenIf()const   { return m_token_if; }
        [[nodiscard]] inline const Token*    getTokenElse()const   { return m_token_else; }
                      void                   getLastInstructions(std::vector<InstructionNode*>& out) override;

    private:
        Token* m_token_if;
        Token* m_token_else;

    // reflect class using mirror
    MIRROR_CLASS(ConditionalStructNode)
    (
        MIRROR_PARENT(CodeBlockNode)
    )
    };
}
