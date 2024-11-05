#include "SwitchBehavior.h"

using namespace ndbl;

void SwitchBehavior::init(Node* node, size_t branch_count)
{
    VERIFY( node != nullptr, "Please provide this Node*" );
    VERIFY( 1 < branch_count && branch_count <= BRANCH_MAX, "branch_count is out of range");

    m_branch_count = branch_count;

    // Add a FLOW_IN slot accepting N inputs
    node->add_slot(node->value(), SlotFlag_FLOW_IN  , Slot::MAX_CAPACITY);

    // Add an inner scope, and attaching N (1 per branch) child scopes and FLOW_OUT slots
    node->init_internal_scope();
    ComponentFactory* component_factory = get_component_factory();
    for(size_t branch = 0; branch < m_branch_count; ++branch )
    {
        // child scope
        m_branch_scope[branch] = component_factory->create<Scope>();
        m_branch_scope[branch]->set_name("ChildScope_"+ std::to_string(branch) );
        m_branch_scope[branch]->reset_parent(node->internal_scope(), ScopeFlags_CLEAR_WITH_PARENT );
        node->add_component( m_branch_scope[branch] );

        // FLOW_OUT
        m_flow_out[branch] = node->add_slot(node->value(), SlotFlag_FLOW_OUT, 1, branch);
    }

    // Add i-1 conditions (no condition for the default branch)
    for(size_t branch = 1; branch < m_branch_count; ++branch )
    {
        auto condition_property  = node->add_prop<tools::any>(CONDITION_PROPERTY);
        m_condition_in[branch-1] = node->add_slot(condition_property, SlotFlag_INPUT, 1, branch);
    }
}
