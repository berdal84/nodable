#pragma once
#include "IConditional.h"
#include "Node.h"
#include "Scope.h"
#include "Slot.h"

namespace ndbl
{
    template<size_t BRANCH_COUNT>
    class TConditionalNode : public Node, public IConditional
    {
    protected:
        static_assert( BRANCH_COUNT > 0, "Branch count should be strictly positive");
        std::array<ID8<Slot>, BRANCH_COUNT> m_next_slot_id;
        std::array<ID8<Slot>, BRANCH_COUNT> m_child_slot_id;

    public:
        PoolID<InstructionNode> cond_instr;

        TConditionalNode() = default;
        TConditionalNode( TConditionalNode&&) = default;
        TConditionalNode& operator=( TConditionalNode&&) = default;
        void          init() override;
        PoolID<Scope> get_scope_at(size_t _branch) const override;
        Slot&         get_child_slot_at(size_t _branch) override;
        const Slot&   get_child_slot_at(size_t _branch) const override;
        REFLECT_DERIVED_CLASS()
    };

    template<size_t BRANCH_COUNT>
    void TConditionalNode<BRANCH_COUNT>::init()
    {
        Node::init();

        add_slot( SlotFlag_PREV, SLOT_MAX_CAPACITY );
        add_slot( SlotFlag_PARENT, 1);
        add_slot( SlotFlag_OUTPUT, SLOT_MAX_CAPACITY );

        for(auto branch = 0; branch < BRANCH_COUNT; ++branch )
        {
            m_next_slot_id[branch]  = add_slot( SlotFlag_NEXT, 1, branch );
            m_child_slot_id[branch] = add_slot( SlotFlag_CHILD, 1, branch );
        }

        auto condition_property_id = add_prop<PoolID<Node>>(CONDITION_PROPERTY, PropertyFlag_VISIBLE);
        add_slot( SlotFlag::SlotFlag_INPUT, 1, condition_property_id );
    }

    template<size_t BRANCH_COUNT>
    const Slot& TConditionalNode<BRANCH_COUNT>::get_child_slot_at( size_t _branch ) const
    {
        FW_ASSERT(_branch < BRANCH_COUNT )
        return m_slots[ (u8_t)m_child_slot_id[_branch] ];
    }

    template<size_t BRANCH_COUNT>
    PoolID<Scope> TConditionalNode<BRANCH_COUNT>::get_scope_at( size_t _branch ) const
    {
        FW_ASSERT(_branch < BRANCH_COUNT )
        const Slot& slot = m_slots[ (u8_t)m_child_slot_id[_branch] ];
        FW_ASSERT(slot);
        if ( SlotRef adjacent_slot = slot.first_adjacent() )
        {
            return adjacent_slot.node->get_component<Scope>();
        }
        return {};
    }

    template<size_t BRANCH_COUNT>
    Slot& TConditionalNode<BRANCH_COUNT>::get_child_slot_at( size_t _branch )
    {
        return const_cast<Slot&>(const_cast<const TConditionalNode<BRANCH_COUNT>*>(this)->get_child_slot_at(_branch));
    }
}

static_assert(std::is_move_assignable_v<ndbl::TConditionalNode<2>>, "Should be move assignable");
static_assert(std::is_move_constructible_v<ndbl::TConditionalNode<2>>, "Should be move constructible");