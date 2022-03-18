#pragma once

#include <memory>
#include <nodable/core/Token.h>
#include <nodable/core/Node.h> // base class
#include <nodable/core/IConditionalStruct.h> // interface

namespace Nodable
{

    // forward declarations
    class Scope;

    /**
     * @brief Class to represent a conditional iterative structure: for( init_state, condition_expr, iterate_expr ) { ... }
     */
    class ForLoopNode
            : public Node
            , public IConditionalStruct {
    public:
        ForLoopNode();
        ~ForLoopNode() = default;

        inline void            set_token_for(std::shared_ptr<Token> _token) { m_token_for = _token; }
        inline std::shared_ptr<const Token> get_token_for()const   { return m_token_for; }
        Member*                get_init_expr()const { return m_props.get(k_forloop_initialization_member_name); }
        Member*                get_iter_expr()const { return m_props.get(k_forloop_iteration_member_name); }

        // override IConditionalStruct
        Member* condition_member()const override { return m_props.get(k_condition_member_name);}
        Scope*  get_condition_true_branch()const override;
        Scope*  get_condition_false_branch()const override;

    private:
        std::shared_ptr<Token> m_token_for;

        R_DERIVED(ForLoopNode)
            R_EXTENDS(Node)
            R_EXTENDS(IConditionalStruct)
        R_END
    };
}
