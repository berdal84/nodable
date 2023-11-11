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
        static_assert( BRANCH_COUNT > 1, "Branch count should be strictly greater than 1");
        std::array<ID8<Slot>, BRANCH_COUNT>    m_next_slot_id;
        std::array<ID8<Slot>, BRANCH_COUNT>    m_child_slot_id;
        std::array<ID8<Slot>, BRANCH_COUNT-1>  m_condition_slot_id; // branch_FALSE has no condition

    public:
        TConditionalNode() = default;
        TConditionalNode( TConditionalNode&&) = default;
        TConditionalNode& operator=( TConditionalNode&&) = default;
        void          init() override;
        PoolID<Scope> get_scope_at(Branch _branch) const override;
        Slot&         get_child_slot_at(Branch _branch) override;
        const Slot&   get_child_slot_at(Branch _branch) const override;
        PoolID<Node>  get_condition(Branch = Branch_TRUE)const override;
        const Slot&   get_condition_slot(Branch = Branch_TRUE) const override;
        Slot&         get_condition_slot(Branch = Branch_TRUE) override;
        REFLECT_DERIVED_CLASS()
    };
    template<size_t BRANCH_COUNT>
    Slot& TConditionalNode<BRANCH_COUNT>::get_condition_slot( Branch _branch )
    {
        return const_cast<Slot&>( const_cast<const TConditionalNode<BRANCH_COUNT>*>(this)->get_condition_slot(_branch) );
    }

    template<size_t BRANCH_COUNT>
    const Slot& TConditionalNode<BRANCH_COUNT>::get_condition_slot( Branch _branch ) const
    {
        FW_ASSERT( _branch > 0 && _branch < BRANCH_COUNT ) // branch_FALSE has no condition
        return get_slot_at( m_condition_slot_id.at(_branch - 1) );
    }

    template<size_t BRANCH_COUNT>
    PoolID<Node> TConditionalNode<BRANCH_COUNT>::get_condition( Branch _branch ) const
    {
        return get_condition_slot(_branch).first_adjacent().node;
    }

    template<size_t BRANCH_COUNT>
    void TConditionalNode<BRANCH_COUNT>::init()
    {
        static_assert( BRANCH_COUNT == 2, "Currently only implemented for 2 branches" );

        Node::init();

        add_slot( SlotFlag_OUTPUT, SLOT_MAX_CAPACITY );

        // A default NEXT branch exists.
        m_next_slot_id[Branch_FALSE] = find_slot( SlotFlag_NEXT )->id;
        m_next_slot_id[Branch_TRUE]  = add_slot( SlotFlag_NEXT, 1, Branch_TRUE );

        // No condition needed for the first slot
        auto condition_property = add_prop<PoolID<Node>>(CONDITION_PROPERTY, PropertyFlag_VISIBLE);
        m_condition_slot_id[0] = add_slot( SlotFlag::SlotFlag_INPUT, 1, condition_property );

        m_child_slot_id[Branch_FALSE] = add_slot( SlotFlag_CHILD, 1, Branch_FALSE );
        m_child_slot_id[Branch_TRUE]  = add_slot( SlotFlag_CHILD, 1, Branch_TRUE );

    }

    template<size_t BRANCH_COUNT>
    const Slot& TConditionalNode<BRANCH_COUNT>::get_child_slot_at( Branch _branch ) const
    {
        FW_ASSERT(_branch < BRANCH_COUNT )
        return m_slots[ (u8_t)m_child_slot_id[_branch] ];
    }

    template<size_t BRANCH_COUNT>
    PoolID<Scope> TConditionalNode<BRANCH_COUNT>::get_scope_at( Branch _branch ) const
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
    Slot& TConditionalNode<BRANCH_COUNT>::get_child_slot_at( Branch _branch )
    {
        return const_cast<Slot&>(const_cast<const TConditionalNode<BRANCH_COUNT>*>(this)->get_child_slot_at(_branch));
    }
}

static_assert(std::is_move_assignable_v<ndbl::TConditionalNode<2>>, "Should be move assignable");
static_assert(std::is_move_constructible_v<ndbl::TConditionalNode<2>>, "Should be move constructible");