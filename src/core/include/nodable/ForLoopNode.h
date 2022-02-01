#pragma once

#include <nodable/Token.h>
#include <nodable/Node.h> // base class
#include <nodable/AbstractConditionalStruct.h> // interface

namespace Nodable
{

    // forward declarations
    class Scope;

    /**
     * @brief Class to represent a conditional iterative structure: for( init_state, condition_expr, iterate_expr ) { ... }
     */
    class ForLoopNode
            : public Node
            , public AbstractConditionalStruct {
    public:
        ForLoopNode();
        ~ForLoopNode() = default;

        inline void            set_token_for(Token* _token) { m_token_for = _token; }
        inline const Token*    get_token_for()const   { return m_token_for; }
        Member*                get_init_expr()const { return m_props.get("init"); }
        Member*                get_iter_expr()const { return m_props.get("iter"); }

        // override AbstractConditionalStruct
        void                   set_condition(Member *_value) const override { get_condition()->set(_value); }
        Member*                get_condition()const override { return m_props.get("condition"); }
        Scope*        get_condition_true_branch()const override;
        Scope*        get_condition_false_branch()const override;

    private:
        Token* m_token_for;

        REFLECT_DERIVED(ForLoopNode)
            REFLECT_EXTENDS(Node)
            REFLECT_EXTENDS(AbstractConditionalStruct)
        REFLECT_END
    };
}
