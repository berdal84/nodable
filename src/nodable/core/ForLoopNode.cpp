#include "ForLoopNode.h"
#include "core/Scope.h"
#include "core/InstructionNode.h"

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<ForLoopNode>("ForLoopNode")
        .extends<Node>()
        .extends<IConditionalStruct>();
}

ForLoopNode::ForLoopNode()
{
    add_prop<ID<Node>>(k_interative_init_property_name, Visibility::Always, Way::Way_In);  // for ( <here> ;   ..    ;   ..   ) { ... }
    add_prop<ID<Node>>(k_conditional_cond_property_name, Visibility::Always, Way::Way_In); // for (   ..   ; <here>  ;   ..   ) { ... }
    add_prop<ID<Node>>(k_interative_iter_property_name, Visibility::Always, Way::Way_In);  // for (   ..   ;   ..    ; <here> ) { ... }
}

ID<Scope> ForLoopNode::get_condition_true_scope() const
{
    return !successors.empty() ? successors[0]->get_component<Scope>() : ID<Scope>{};
}

ID<Scope> ForLoopNode::get_condition_false_scope() const
{
    return successors.size() > 1 ? successors[1]->get_component<Scope>() : ID<Scope>{};
}