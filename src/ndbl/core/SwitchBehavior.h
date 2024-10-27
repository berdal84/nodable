#pragma once
#include "Node.h"
#include "Scope.h"
#include "Slot.h"
#include "ComponentFactory.h"

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
     */
    class SwitchBehavior
    {
    public:
        static constexpr size_t BRANCH_MAX = 2;

        Slot*         branch_out(Branch branch)                       { ASSERT(branch < m_branch_count); return m_flow_out[branch]; }
        const Slot*   branch_out(Branch branch) const                 { ASSERT(branch < m_branch_count); return m_flow_out[branch]; }
        const Node*   condition(Branch branch = Branch_TRUE) const    { ASSERT(Branch_FALSE < branch && branch < m_branch_count); return m_condition_in[branch-1]->first_adjacent_node(); }
        Node*         condition(Branch branch = Branch_TRUE)          { ASSERT(Branch_FALSE < branch && branch < m_branch_count); return m_condition_in[branch-1]->first_adjacent_node(); }
        const Slot*   condition_in(Branch branch = Branch_TRUE) const { ASSERT(Branch_FALSE < branch && branch < m_branch_count); return m_condition_in[branch-1]; }
        Slot*         condition_in(Branch branch = Branch_TRUE)       { ASSERT(Branch_FALSE < branch && branch < m_branch_count); return m_condition_in[branch-1]; }
        void          init(Node* node, size_t branch_count)
        {
            ASSERT( node != nullptr );
            ASSERT( branch_count > 1 ); // always required, what's the point to have 1 branch...?
            ASSERT( branch_count <= BRANCH_MAX );

            m_branch_count = branch_count;

            // add single flow_in
            node->add_slot(node->value(), SlotFlag_FLOW_IN  , Slot::MAX_CAPACITY);

            ComponentFactory* component_factory = get_component_factory();
            node->init_inner_scope();
             for(size_t branch = 0; branch < m_branch_count; ++branch )
            {
                m_branch_scope[branch] = component_factory->create<Scope>();
                m_branch_scope[branch]->set_name("#"+ std::to_string(branch) );
                m_branch_scope[branch]->reset_parent( node->inner_scope() );
                node->add_component( m_branch_scope[branch] );

                m_flow_out[branch]     = node->add_slot(node->value(), SlotFlag_FLOW_OUT, 1, branch);
            }

            // add i-1 conditions
            for(size_t branch = 1; branch < m_branch_count; ++branch )
            {
                // -1 => No condition needed for the first slot
                auto condition_property  = node->add_prop<tools::any>(CONDITION_PROPERTY);
                m_condition_in[branch-1] = node->add_slot(condition_property, SlotFlag_INPUT, 1, branch);
            }
        }
    private:

        size_t                           m_branch_count = 0;
        std::array<Scope*, BRANCH_MAX>   m_branch_scope;
        std::array<Slot*, BRANCH_MAX>    m_flow_out;
        std::array<Slot*, BRANCH_MAX-1>  m_condition_in; // branch_FALSE has no condition
    };
}
