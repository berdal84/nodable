#pragma once
#include "Node.h"
#include "Scope.h"
#include "Slot.h"

namespace ndbl
{
    typedef size_t Branch;
    enum Branch_ : size_t
    {
        Branch_FALSE   = 0,
        Branch_TRUE    = 1,
    };

    /**
     * Adds a conditional behavior to a given Node
     * @tparam BRANCH_COUNT the possible branch count (ex: for an if we have 2 branches, for a switch it can be N+1)
     */
    template<size_t BRANCH_COUNT>
    class TConditional
    {
    public:
        static_assert( BRANCH_COUNT > 1, "Branch count should be strictly greater than 1");

        void          init(Node* node);
        Scope*        get_scope_at(Branch _branch) const;
        Slot*         get_child_slot_at(Branch _branch);
        const Slot*   get_child_slot_at(Branch _branch) const;
        const Node*   get_condition(Branch)const;
        Node*         get_condition(Branch);
        const Slot*   get_condition_slot(Branch) const;
        Slot*         get_condition_slot(Branch);

    private:
        std::array<Slot*, BRANCH_COUNT>    m_next_slot;
        std::array<Slot*, BRANCH_COUNT>    m_child_slot;
        std::array<Slot*, BRANCH_COUNT-1>  m_condition_slot; // branch_FALSE has no condition
    };

    template<size_t BRANCH_COUNT>
    Slot* TConditional<BRANCH_COUNT>::get_condition_slot(Branch _branch )
    {
        return const_cast<Slot*>( const_cast<const TConditional<BRANCH_COUNT>*>(this)->get_condition_slot(_branch) );
    }

    template<size_t BRANCH_COUNT>
    const Slot* TConditional<BRANCH_COUNT>::get_condition_slot(Branch _branch ) const
    {
        VERIFY(_branch != Branch_FALSE, "Branch_FALSE has_flags no condition, use Branch_TRUE or any number greater than 0" );
        VERIFY(_branch < BRANCH_COUNT, "branch does not exist" );
        return m_condition_slot.at(_branch - 1);
    }

    template<size_t BRANCH_COUNT>
    const Node* TConditional<BRANCH_COUNT>::get_condition(Branch _branch ) const
    {
        // Try to return the adjacent node connected to this branch
        Slot* adjacent = get_condition_slot(_branch)->first_adjacent();
        if( adjacent != nullptr)
            return adjacent->node;
        return nullptr;
    }

    template<size_t BRANCH_COUNT>
    Node* TConditional<BRANCH_COUNT>::get_condition(Branch _branch )
    {
        return const_cast<Node*>( const_cast<TConditional*>(this)->get_condition(_branch) );
    }

    template<size_t BRANCH_COUNT>
    void TConditional<BRANCH_COUNT>::init(Node* node)
    {
        static_assert( BRANCH_COUNT == 2, "Currently only implemented for 2 branches" );
        ASSERT(node != nullptr);

        node->add_slot(node->value(), SlotFlag_PARENT, 1);
        node->add_slot(node->value(), SlotFlag_PREV  , Slot::MAX_CAPACITY);
        node->add_component( new Scope() );

        // A default NEXT branch exists.
        m_next_slot[Branch_FALSE] = node->add_slot(node->value(), SlotFlag_NEXT, 1, Branch_FALSE);
        m_next_slot[Branch_TRUE]  = node->add_slot(node->value(), SlotFlag_NEXT, 1, Branch_TRUE);

        // No condition needed for the first slot
        auto condition_property = node->add_prop<Node*>(CONDITION_PROPERTY);
        m_condition_slot[0]     = node->add_slot(condition_property, SlotFlag_INPUT, 1);

        m_child_slot[Branch_FALSE] = node->add_slot(node->value(), SlotFlag_CHILD, 1, Branch_FALSE);
        m_child_slot[Branch_TRUE]  = node->add_slot(node->value(), SlotFlag_CHILD, 1, Branch_TRUE);
    }

    template<size_t BRANCH_COUNT>
    const Slot* TConditional<BRANCH_COUNT>::get_child_slot_at(Branch _branch ) const
    {
        ASSERT(_branch < BRANCH_COUNT );
        return m_child_slot[_branch];
    }

    template<size_t BRANCH_COUNT>
    Scope* TConditional<BRANCH_COUNT>::get_scope_at(Branch _branch ) const
    {
        ASSERT(_branch < BRANCH_COUNT );
        if ( Slot* adjacent_slot = m_child_slot[_branch]->first_adjacent() )
        {
            return adjacent_slot->node->get_component<Scope>();
        }
        return {};
    }

    template<size_t BRANCH_COUNT>
    Slot* TConditional<BRANCH_COUNT>::get_child_slot_at(Branch _branch )
    {
        return const_cast<Slot*>(const_cast<const TConditional<BRANCH_COUNT>*>(this)->get_child_slot_at(_branch));
    }
}
