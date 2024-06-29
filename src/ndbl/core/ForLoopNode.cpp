#include "ForLoopNode.h"
#include "Node.h"
#include "GraphUtil.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<ForLoopNode>("ForLoopNode")
        .extends<Node>()
        .extends<IConditional>();
}

void ForLoopNode::init(const std::string& _name)
{
    // add initialization property and slot
    auto init_id = add_prop<Node*>(INITIALIZATION_PROPERTY );
    m_initialization_slot = add_slot( SlotFlag_INPUT, 1, init_id);

    // indirectly add condition property and slot
    TConditionalNode::init(NodeType_BLOCK_FOR_LOOP, _name);

    // add iteration property and slot
    auto iter_id = add_prop<Node*>(ITERATION_PROPERTY );
    m_iteration_slot = add_slot( SlotFlag_INPUT, 1, iter_id);
}

Slot& ForLoopNode::iteration_slot()
{
    ASSERT(m_iteration_slot)
    return *m_iteration_slot;
}

Slot& ForLoopNode::initialization_slot()
{
    ASSERT(m_initialization_slot)
    return *m_initialization_slot;
}

const Slot& ForLoopNode::iteration_slot() const
{
    ASSERT(m_iteration_slot)
    return *m_iteration_slot;
}

const Slot& ForLoopNode::initialization_slot() const
{
    ASSERT(m_initialization_slot)
    return *m_initialization_slot;
}