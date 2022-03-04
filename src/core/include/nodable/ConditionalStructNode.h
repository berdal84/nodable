#pragma once

#include <memory>
#include <nodable/Token.h>
#include <nodable/Node.h>
#include <nodable/IScope.h>
#include <nodable/IConditionalStruct.h>

namespace Nodable
{

    /**
     * @brief Class to represent a conditional structure ( IF/ELSE )
     */
    class ConditionalStructNode
            : public Node
            , public IConditionalStruct
    {
    public:
        ConditionalStructNode();
        ~ConditionalStructNode() = default;

        inline void            set_token_if(std::shared_ptr<Token> token) { m_token_if = token; }
        inline void            set_token_else(std::shared_ptr<Token> token) { m_token_else = token; }
        inline std::shared_ptr<const Token> get_token_if()const   { return m_token_if; }
        inline std::shared_ptr<const Token> get_token_else()const { return m_token_else; }

        // override AbstractConditionalStruct
        Scope*        get_condition_true_branch()const override;
        Scope*        get_condition_false_branch()const override;
        Member*       condition_member()const override { return m_props.get("condition"); }

    private:
        std::shared_ptr<Token> m_token_if;
        std::shared_ptr<Token> m_token_else;

        R_DERIVED(ConditionalStructNode)
         R_EXTENDS(Node)
         R_EXTENDS(IConditionalStruct)
        R_END
    };
}