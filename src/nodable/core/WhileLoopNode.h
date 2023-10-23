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
        Token                   token_while;
        PoolID<InstructionNode> cond_instr;

        WhileLoopNode() = default;
        WhileLoopNode(WhileLoopNode&&) = default;
        WhileLoopNode& operator=(WhileLoopNode&&) = default;
        ~WhileLoopNode() = default;

        void init() override;

        // implements IConditionalStruct (which is already documented)

        PoolID<Scope>  get_scope_at(size_t _pos) const override;
        Slot&          get_child_slot_at(size_t _pos) override;
        const Slot&    get_child_slot_at(size_t _pos) const override;

    private:
        std::array<ID8<Slot>, 2> m_next_slot;
        std::array<ID8<Slot>, 2> m_child_slot;
        REFLECT_DERIVED_CLASS()
    };
}
