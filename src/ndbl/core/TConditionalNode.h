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
        std::array<Slot*, BRANCH_COUNT>    m_next_slot;
        std::array<Slot*, BRANCH_COUNT>    m_child_slot;
        std::array<Slot*, BRANCH_COUNT-1>  m_condition_slot; // branch_FALSE has no condition

    public:
        TConditionalNode() = default;
        TConditionalNode( TConditionalNode&&) = default;
        TConditionalNode& operator=( TConditionalNode&&) = default;
        void          init() override;
        Scope*        get_scope_at(Branch _branch) const override;
        Slot&         get_child_slot_at(Branch _branch) override;
        const Slot&   get_child_slot_at(Branch _branch) const override;
        Node*         get_condition(Branch)const override;
        const Slot&   get_condition_slot(Branch) const override;
        Slot&         get_condition_slot(Branch) override;
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
        EXPECT( _branch != Branch_FALSE, "Branch_FALSE has no condition, use Branch_TRUE or any number greater than 0" )
        EXPECT( _branch < BRANCH_COUNT, "branch does not exist" )
        return *m_condition_slot.at(_branch - 1);
    }

    template<size_t BRANCH_COUNT>
    Node* TConditionalNode<BRANCH_COUNT>::get_condition( Branch _branch ) const
    {
        return get_condition_slot(_branch).first_adjacent()->get_node();
    }

    template<size_t BRANCH_COUNT>
    void TConditionalNode<BRANCH_COUNT>::init()
    {
        static_assert( BRANCH_COUNT == 2, "Currently only implemented for 2 branches" );

        Node::init();

        add_slot( SlotFlag_OUTPUT, Slot::MAX_CAPACITY );

        // A default NEXT branch exists.
        m_next_slot[Branch_FALSE] = find_slot(SlotFlag_NEXT );
        m_next_slot[Branch_TRUE]  = add_slot(SlotFlag_NEXT, 1, Branch_TRUE );

        // No condition needed for the first slot
        auto condition_property = add_prop<Node*>(CONDITION_PROPERTY, PropertyFlag_VISIBLE);
        m_condition_slot[0] = add_slot(SlotFlag::SlotFlag_INPUT, 1, condition_property );

        m_child_slot[Branch_FALSE] = add_slot(SlotFlag_CHILD, 1, Branch_FALSE );
        m_child_slot[Branch_TRUE]  = add_slot(SlotFlag_CHILD, 1, Branch_TRUE );

    }

    template<size_t BRANCH_COUNT>
    const Slot& TConditionalNode<BRANCH_COUNT>::get_child_slot_at( Branch _branch ) const
    {
        ASSERT(_branch < BRANCH_COUNT )
        return *m_child_slot[_branch];
    }

    template<size_t BRANCH_COUNT>
    Scope* TConditionalNode<BRANCH_COUNT>::get_scope_at( Branch _branch ) const
    {
        ASSERT(_branch < BRANCH_COUNT )
        if ( Slot* adjacent_slot = m_child_slot[_branch]->first_adjacent() )
        {
            return adjacent_slot->get_node()->get_component<Scope>();
        }
        return {};
    }

    template<size_t BRANCH_COUNT>
    Slot& TConditionalNode<BRANCH_COUNT>::get_child_slot_at( Branch _branch )
    {
        return const_cast<Slot&>(const_cast<const TConditionalNode<BRANCH_COUNT>*>(this)->get_child_slot_at(_branch));
    }
}
