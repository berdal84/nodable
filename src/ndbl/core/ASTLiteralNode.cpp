#include "ASTLiteralNode.h"
#include "Slot.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    type::Initializer<ASTLiteralNode>("LiteralNode").extends<ASTNode>();
}

void ASTLiteralNode::init(const TypeDescriptor* _type, const std::string& _name)
{
    ASTNode::init(ASTNodeType_LITERAL, _name);
    m_value->set_type(_type);

    add_slot(m_value, SlotFlag_PARENT, 1);
    add_slot(m_value, SlotFlag_NEXT  , 1);
    add_slot(m_value, SlotFlag_PREV  , Slot::MAX_CAPACITY);
    add_slot(m_value, SlotFlag_OUTPUT, 1);
}

