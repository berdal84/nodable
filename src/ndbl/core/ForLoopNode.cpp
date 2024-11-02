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
    auto* init_prop = add_prop<any>(INITIALIZATION_PROPERTY );
    m_initialization_slot = add_slot(init_prop, SlotFlag_INPUT, 1);

    // add conditional-related properties and slots
    SwitchBehavior::init(this, 2);
    Property* condition_prop = SwitchBehavior::condition_in(Branch_TRUE)->property;

    // add iteration property and slot
    auto iter_prop = add_prop<any>(ITERATION_PROPERTY );
    m_iteration_slot = add_slot(iter_prop, SlotFlag_INPUT, 1);
}