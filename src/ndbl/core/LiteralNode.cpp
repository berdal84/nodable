#include "LiteralNode.h"
#include "Slot.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INITIALIZER
(
    DEFINE_REFLECT(LiteralNode).extends<Node>();
)

void LiteralNode::init(const TypeDescriptor* _type, const std::string& _name)
{
    Node::init(NodeType_LITERAL, _name);
    m_value->set_type(_type);

    add_slot(m_value, SlotFlag_FLOW_OUT , 1);
    add_slot(m_value, SlotFlag_FLOW_IN  , Slot::MAX_CAPACITY);
    add_slot(m_value, SlotFlag_OUTPUT   , 1);
}

