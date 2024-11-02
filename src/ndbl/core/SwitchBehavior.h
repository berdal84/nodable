#pragma once
#include "Node.h"
#include "Scope.h"
#include "Slot.h"
#include "ComponentFactory.h"

namespace ndbl
{
    enum Branch : size_t
    {
        Branch_FALSE = 0,
        Branch_TRUE  = 1,
    };

    /**
     * Adds a conditional behavior to a given Node.
     */
    class SwitchBehavior
    {
    public:
        static constexpr size_t BRANCH_MAX = 2;

        void          init(Node* node, size_t branch_count);
        Slot*         branch_out(Branch branch)                       { ASSERT(branch < m_branch_count); return m_flow_out[branch]; }
        const Slot*   branch_out(Branch branch) const                 { ASSERT(branch < m_branch_count); return m_flow_out[branch]; }
        const Node*   condition(Branch branch = Branch_TRUE) const    { ASSERT(Branch_FALSE < branch && branch < m_branch_count); return m_condition_in[branch-1]->first_adjacent_node(); }
        Node*         condition(Branch branch = Branch_TRUE)          { ASSERT(Branch_FALSE < branch && branch < m_branch_count); return m_condition_in[branch-1]->first_adjacent_node(); }
        const Slot*   condition_in(Branch branch = Branch_TRUE) const { ASSERT(Branch_FALSE < branch && branch < m_branch_count); return m_condition_in[branch-1]; }
        Slot*         condition_in(Branch branch = Branch_TRUE)       { ASSERT(Branch_FALSE < branch && branch < m_branch_count); return m_condition_in[branch-1]; }
    private:

        size_t                           m_branch_count = 0;
        std::array<Scope*, BRANCH_MAX>   m_branch_scope;
        std::array<Slot*, BRANCH_MAX>    m_flow_out;
        std::array<Slot*, BRANCH_MAX-1>  m_condition_in; // branch_FALSE has no condition
    };
}
