#pragma once

#include <memory>
#include "Token.h"
#include "Node.h" // base class
#include "IConditionalStruct.h" // interface

namespace ndbl
{
    // forward declarations
    class Scope;
    class InstructionNode;

    /**
     * @class Represent a conditional and iterative structure "for"
     * for( init_state, condition_expr, iterate_expr ) { ... }
     */
    class ForLoopNode
        : public Node
        , public IConditionalStruct {
    public:
        ForLoopNode();
        ForLoopNode(ForLoopNode&& other) = default;
        ForLoopNode& operator=(ForLoopNode&& other) = default;
        ~ForLoopNode() = default;

        Token token_for;
        ID<InstructionNode> init_instr;
        ID<InstructionNode> cond_instr;
        ID<InstructionNode> iter_instr;

        Property*  get_init_expr()const { return get_prop(k_interative_init_property_name); }
        Property*  get_iter_expr()const { return get_prop(k_interative_iter_property_name); }

        // implements IConditionalStruct (which is already documented)

        Property*  condition_property()const override { return get_prop(k_conditional_cond_property_name);}
        ID<Scope>  get_condition_true_scope()const override;
        ID<Scope>  get_condition_false_scope()const override;

        REFLECT_DERIVED_CLASS()
    };
}
