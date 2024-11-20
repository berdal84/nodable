#include "SwitchBehavior.h"

using namespace ndbl;

void SwitchBehavior::init(Node* node, size_t branch_count)
{
    VERIFY( node != nullptr, "Please provide this Node*" );
    VERIFY( 1 < branch_count && branch_count <= BRANCH_MAX, "branch_count is out of range");

    m_branch_count = branch_count;

    // Add a FLOW_IN slot accepting N inputs
    node->add_slot(node->value(), SlotFlag_FLOW_IN  , Slot::MAX_CAPACITY);

    // Add an inner scope (some sub scopes) and FLOW_OUT slots
    node->init_internal_scope( branch_count );
    for(size_t branch = 0; branch < branch_count; ++branch )
    {
        m_flow_out[branch] = node->add_slot(node->value(), SlotFlag_FLOW_OUT, 1, branch);
    }

    // Add i-1 conditions (no condition for the default branch)
    for(size_t branch = 1; branch < branch_count; ++branch )
    {
        auto condition_property  = node->add_prop<tools::any>(CONDITION_PROPERTY);
        m_condition_in[branch-1] = node->add_slot(condition_property, SlotFlag_INPUT, 1, branch);
    }
}
