#pragma once

#include <memory>
#include "fw/core/reflection/reflection"

#include "Token.h"
#include "Node.h" // base class
#include "IConditionalStruct.h" // interface

namespace ndbl
{
    // forward declarations
    class Scope;
    class InstructionNode;

    /**
     * @class Represent a conditional and iterative structure "while"
     * while( condition_expr ) {
     *   // do something
     * }
     */
    class WhileLoopNode
        : public Node
        , public IConditionalStruct {
    public:
        WhileLoopNode();
        WhileLoopNode(WhileLoopNode&&) = default;
        WhileLoopNode& operator=(WhileLoopNode&&) = default;
        ~WhileLoopNode() = default;

        Token               token_while;
        PoolID<InstructionNode> cond_instr;

        // implements IConditionalStruct (which is already documented)

        const Property* condition_property()const override;
        PoolID<Scope> get_condition_true_scope()const override;
        PoolID<Scope> get_condition_false_scope()const override;

        REFLECT_DERIVED_CLASS()
    };
}
