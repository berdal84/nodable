#include "ASTForLoopNode.h"
#include "ASTNode.h"
#include "ASTUtils.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    type::Initializer<ASTForLoopNode>("ForLoopNode").extends<ASTNode>();
}

void ASTForLoopNode::init(const std::string& _name)
{
    ASTNode::init(ASTNodeType_BLOCK_FOR_LOOP, _name);

    m_wrapped_conditional.init(this);

    // add initialization property and slot
    auto* init_prop = add_prop<ASTNode*>(INITIALIZATION_PROPERTY );
    m_initialization_slot = add_slot(init_prop, SlotFlag_INPUT, 1);

    // add iteration property and slot
    auto iter_prop = add_prop<ASTNode*>(ITERATION_PROPERTY );
    m_iteration_slot = add_slot(iter_prop, SlotFlag_INPUT, 1);
}

Slot& ASTForLoopNode::iteration_slot()
{
    ASSERT(m_iteration_slot)
    return *m_iteration_slot;
}

Slot& ASTForLoopNode::initialization_slot()
{
    ASSERT(m_initialization_slot)
    return *m_initialization_slot;
}

const Slot& ASTForLoopNode::iteration_slot() const
{
    ASSERT(m_iteration_slot)
    return *m_iteration_slot;
}

const Slot& ASTForLoopNode::initialization_slot() const
{
    ASSERT(m_initialization_slot)
    return *m_initialization_slot;
}