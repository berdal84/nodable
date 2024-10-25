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
    init_prop->token().suffix_push_back(";");
    m_initialization_slot = add_slot(init_prop, SlotFlag_INPUT, 1);

    // add conditional-related properties and slots
    m_wrapped_conditional.init(this);
    Property* condition_prop = m_wrapped_conditional.get_condition_slot(Branch_TRUE)->property;
    condition_prop->token().suffix_push_back(";");

    // add iteration property and slot
    auto iter_prop = add_prop<any>(ITERATION_PROPERTY );
    m_iteration_slot = add_slot(iter_prop, SlotFlag_INPUT, 1);
    // iter_prop->token().suffix_push_back(";"); unnecessary
}