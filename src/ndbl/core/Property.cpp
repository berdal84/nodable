#include "Property.h"

#include "PropertyBag.h"
#include "VariableNode.h"
#include "tools/core/memory/memory.h"
#include "ndbl/core/language/Nodlang.h"

using namespace ndbl;
using namespace tools;

void Property::init(const TypeDescriptor* _type, PropertyFlags _flags, Node* _owner, const char* _name)
{
    VERIFY(m_type == nullptr, "must be initialized once");
    VERIFY(_type != nullptr, "type can't be nullptr");
    m_type  = _type;
    m_flags = _flags;
    m_owner = _owner;
    m_name  = _name;

    // Ensure token matches with Property type
    init_token();
}

void Property::init_token()
{
    const Nodlang* language = get_language();

    // Convert m_type to a Token_t
    Token_t token_type = language->to_literal_token( m_type );
    VERIFY(token_type != Token_t::none, "This token is not handled");

    // Serialize the default value for this Token_t
    std::string buffer = language->serialize_token_t(token_type);

    m_token = {
        token_type,
        buffer
    };
}

void Property::digest(Property* _property)
{
    m_token = std::move( _property->m_token );
}

bool Property::is_type(const TypeDescriptor* other) const
{
    return m_type->equals( other );
}

void Property::set_type(const tools::TypeDescriptor* type)
{
    bool change = type != m_type;
    m_type = type;
    if (change)
        init_token();
}
