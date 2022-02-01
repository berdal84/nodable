#pragma once

#include <nodable/Token.h>
#include <nodable/Node.h> // base class
#include <nodable/AbstractScope.h> // interface
#include <nodable/AbstractConditionalStruct.h> // interface

namespace Nodable
{

    /**
     * @brief Class to represent a conditional structure ( IF/ELSE )
     */
    class ConditionalStructNode
            : public Node // base class
            , public AbstractConditionalStruct // interface
    {
    public:
        ConditionalStructNode();
        ~ConditionalStructNode() = default;

        inline void            set_token_if(Token *token) { m_token_if = token; }
        inline void            set_token_else(Token *token) { m_token_else = token; }
        inline const Token*    get_token_if()const   { return m_token_if; }
        inline const Token*    get_token_else()const   { return m_token_else; }

        // override AbstractConditionalStruct
        void                   set_condition(Member *_value) const override { get_condition()->set(_value); }
        Scope*        get_condition_true_branch()const override;
        Scope*        get_condition_false_branch()const override;
        Member*                get_condition()const override { return m_props.get("condition"); }

    private:
        Token*         m_token_if;
        Token*         m_token_else;

        REFLECT_DERIVED(ConditionalStructNode)
         REFLECT_EXTENDS(Node)
         REFLECT_EXTENDS(AbstractConditionalStruct)
        REFLECT_END
    };
}