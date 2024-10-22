#include "ForLoopNode.h"
#include "Node.h"
#include "Utils.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    type::Initializer<ForLoopNode>("ForLoopNode").extends<Node>();
}

void ForLoopNode::init(const std::string& _name)
{
    Node::init(NodeType_BLOCK_FOR_LOOP, _name);

    // add initialization property and slot
    auto* init_prop = add_prop<Node*>(INITIALIZATION_PROPERTY );
    m_initialization_slot = add_slot(init_prop, SlotFlag_INPUT, 1);

    // add conditional-related properties and slots
    m_wrapped_conditional.init(this);

    // add iteration property and slot
    auto iter_prop = add_prop<Node*>(ITERATION_PROPERTY );
    m_iteration_slot = add_slot(iter_prop, SlotFlag_INPUT, 1);
}

Slot& ForLoopNode::iteration_slot()
{
    ASSERT(m_iteration_slot);
    return *m_iteration_slot;
}

Slot& ForLoopNode::initialization_slot()
{
    ASSERT(m_initialization_slot);
    return *m_initialization_slot;
}

const Slot& ForLoopNode::iteration_slot() const
{
    ASSERT(m_iteration_slot);
    return *m_iteration_slot;
}

const Slot& ForLoopNode::initialization_slot() const
{
    ASSERT(m_initialization_slot);
    return *m_initialization_slot;
}