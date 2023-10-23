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
        ForLoopNode() = default;
        ForLoopNode(ForLoopNode&& other) = default;
        ForLoopNode& operator=(ForLoopNode&& other) = default;
        ~ForLoopNode() = default;

        Token token_for;
        PoolID<InstructionNode> init_instr;
        PoolID<InstructionNode> cond_instr;
        PoolID<InstructionNode> iter_instr;

        void            init() override;

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
