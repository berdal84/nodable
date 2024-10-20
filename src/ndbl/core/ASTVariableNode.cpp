#include "ASTVariableNode.h"
#include "Property.h"
#include "ASTScope.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    type::Initializer<ASTVariableNode>("VariableNode").extends<ASTNode>();
}

void ASTVariableNode::init(const tools::TypeDescriptor* _type, const char* _identifier)
{
    // Init node
    ASTNode::init(ASTNodeType_VARIABLE, "Variable");

    // Init identifier property
    m_value->set_type(_type);
    m_value->set_token({TokenType::identifier});
    m_value->token().word_replace(_identifier); // might come from std::string::c_str()

    // Init Slots
    add_slot(m_value, SlotFlag_INPUT, 1); // to connect an initialization expression
    add_slot(m_value, SlotFlag_PARENT, 1);
    add_slot(m_value, SlotFlag_NEXT, 1);
    add_slot(m_value, SlotFlag_PREV, Slot::MAX_CAPACITY);

    m_as_declaration_slot = add_slot(m_value, SlotFlag_OUTPUT, 1); // as declaration
    m_as_reference_slot   = add_slot(m_value, SlotFlag_OUTPUT, Slot::MAX_CAPACITY ); // as reference
}

ASTScope* ASTVariableNode::get_scope()
{
    if (m_scope != nullptr)
        return m_scope->get_component<ASTScope>();
    return {};
}

const ASTScope* ASTVariableNode::get_scope() const
{
    if (m_scope != nullptr)
        return m_scope->get_component<ASTScope>();
    return {};
}

void ASTVariableNode::reset_scope(ASTScope* _scope)
{
    if( _scope != nullptr )
        m_scope = _scope->get_owner();
    else
        m_scope = {};
}
