#include "Property.h"

#include "PropertyBag.h"
#include "VariableNode.h"
#include "tools/core/memory/memory.h"

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
    init_token();
}

void Property::init_token()
{
    // Initialize a default Token
    // it is required to display a default value
    if ( m_type == type::get<double>() )
    {
        m_token = { Token_t::literal_double, "0.0" };
    }
    else if ( m_type == type::get<i16_t>() )
    {
        m_token = { Token_t::keyword_i16, "0" };
    }
    else if ( m_type == type::get<int>() )
    {
        m_token = { Token_t::keyword_int, "0" };
    }
    else if ( m_type == type::get<bool>() )
    {
        m_token = { Token_t::literal_bool, "false" };
    }
    else if ( m_type == type::get<std::string>() )
    {
        m_token = { Token_t::literal_string, "" };
    }
    else if ( m_type == type::get<any>() )
    {
        m_token = { Token_t::any, "" };
    }
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
