#pragma once

#include "Node/CodeBlockNode.h" // base class

namespace Nodable
{
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

        virtual AbstractCodeBlockNode* getNext();
        AbstractCodeBlockNode*         getBranchTrue();
        AbstractCodeBlockNode*         getBranchFalse();
        [[nodiscard]] inline Member*   getCondition()const { return props.get("condition"); }
        [[nodiscard]] inline const Token* getTokenIf()const   { return m_token_if; }
        [[nodiscard]] inline const Token* getTokenElse()const   { return m_token_else; }

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
