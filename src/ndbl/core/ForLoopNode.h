#pragma once

#include "SwitchBehavior.h"
#include "Token.h"
#include <memory>

namespace ndbl
{
    // forward declarations
    struct Scope;
    class InstructionNode;

    /**
     * @class Represent a conditional and iterative structure "for"
     * for( <init_ex>; <condition>; <iteration> ) { ... }
     */
    class ForLoopNode : public Node, public SwitchBehavior
    {
    public:
        REFLECT_DERIVED_CLASS()
        Token token_for = { Token_t::keyword_for };

        void        init(const std::string& _name);
        Slot*       iteration_slot()            { ASSERT(m_iteration_slot); return m_iteration_slot; }
        Slot*       initialization_slot()       { ASSERT(m_initialization_slot); return m_initialization_slot; }
        const Slot* iteration_slot() const      { ASSERT(m_iteration_slot);return m_iteration_slot;}
        const Slot* initialization_slot() const { ASSERT(m_initialization_slot);return m_initialization_slot;}
    private:
        Slot*       m_initialization_slot{nullptr};
        Slot*       m_iteration_slot{nullptr};
    };
}
