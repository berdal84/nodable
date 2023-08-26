#include "ConditionalStructNode.h"
#include "fw/core/reflection/reflection"
#include "Scope.h"
#include "InstructionNode.h"

using namespace ndbl;

REGISTER
{
    using namespace ndbl;
    fw::registration::push_class<ConditionalStructNode>("ConditionalStructNode")
        .extends<Node>()
        .extends<IConditionalStruct>();
}

ConditionalStructNode::ConditionalStructNode()
    : Node()
{
    add_prop<ID<Node>>(k_conditional_cond_property_name, Visibility::Always, Way::Way_In);
}

ID<Scope> ConditionalStructNode::get_condition_true_scope() const
{
    if ( !successors.empty() ) return successors[0]->get_component<Scope>();
    return {};
}

ID<Scope> ConditionalStructNode::get_condition_false_scope() const
{
    if ( successors.size() > 1 ) return successors[1]->get_component<Scope>();
    return {};
}

bool ConditionalStructNode::has_elseif() const
{
    return successors.size() > 1 && successors[1]->get_type()->is_child_of<ConditionalStructNode>();
}

Property * ConditionalStructNode::condition_property() const
{
    return get_prop(k_conditional_cond_property_name);
}