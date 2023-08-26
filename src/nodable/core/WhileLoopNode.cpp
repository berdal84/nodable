#include "WhileLoopNode.h"
#include "core/Scope.h"
#include "InstructionNode.h"

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<WhileLoopNode>("WhileLoopNode")
        .extends<Node>()
        .extends<IConditionalStruct>();
}

WhileLoopNode::WhileLoopNode()
{
    add_prop<ID<Node>>(k_conditional_cond_property_name, Visibility::Always, Way::Way_In); // while ( <here> ) { ... }
}

ID<Scope> WhileLoopNode::get_condition_true_scope() const
{
    if ( !successors.empty() )
        return successors[0]->get_component<Scope>();
    return {};
}

ID<Scope> WhileLoopNode::get_condition_false_scope() const
{
    if ( successors.size() > 1 )
        return successors[1]->get_component<Scope>();
    return {};
}

Property* WhileLoopNode::condition_property() const
{
    return get_prop(k_conditional_cond_property_name);
}
