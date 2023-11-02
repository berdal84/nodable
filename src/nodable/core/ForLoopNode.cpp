#include "ForLoopNode.h"
#include "Node.h"
#include "GraphUtil.h"
#include "core/InstructionNode.h"

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<ForLoopNode>("ForLoopNode")
        .extends<Node>()
        .extends<IConditional>();
}

void ForLoopNode::init()
{
    TConditionalNode::init();

    auto init_id = add_prop<PoolID<Node>>(INITIALIZATION_PROPERTY, PropertyFlag_VISIBLE ); // for ( <here> ;   ..    ;   ..   ) { ... }
    auto iter_id = add_prop<PoolID<Node>>(ITERATION_PROPERTY,      PropertyFlag_VISIBLE ); // for (   ..   ;   ..    ; <here> ) { ... }

    add_slot( SlotFlag_INPUT, 1, init_id);
    add_slot( SlotFlag_INPUT, 1, iter_id);
}
