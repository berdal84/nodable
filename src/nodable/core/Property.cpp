#include "Property.h"

#include "PropertyBag.h"
#include "VariableNode.h"
#include "fw/core/Pool.h"

using namespace ndbl;

Property::Property()
    : m_visibility(Visibility::Default)
    , m_name("Unknown")
    , token(Token::s_null)
    , m_allowed_connection(Way::Default)
{
}

Property::Property(double d)
: Property()
{ m_variant.set(d); }

Property::Property(bool b)
: Property()
{ m_variant.set(b); }

Property::Property(int i)
: Property((double)i)
{}

Property::Property(const char *str)
: Property()
{ m_variant.set(str); }

Property::Property(const std::string &s)
: Property(s.c_str())
{}

void Property::digest(Property* _property)
{
    // Transfer
    m_variant.set(_property->m_variant);
    token = std::move( _property->token );
}

Property* Property::new_with_type(const fw::type *_type, Flags _flags)
{
    Property* property = new Property();
    property->m_variant.ensure_is_type(_type);

    if( _flags & Flags_initialize )
    {
        property->m_variant.ensure_is_initialized();
    }

    if(_flags & Flags_define )
    {
        property->m_variant.flag_defined();
    }

    if(_flags & Flags_reset_value )
    {
        property->m_variant.reset_value();
    }

    return property;
}

std::vector<fw::variant*> Property::get(std::vector<Property*> _in_properties )
{
    std::vector<fw::variant*> result;
    result.reserve(_in_properties.size());
    for(Property* each_property : _in_properties)
    {
        result.push_back(each_property->value() );
    }
    return result;
}

void Property::ensure_is_initialized(bool b)
{
    value()->ensure_is_initialized(false);
}

bool Property::is_ref() const
{
    return m_is_ref;
}

bool Property::is_type_null() const
{
    return get_type()->is<fw::null_t>();
}

bool Property::allows_connection(Way _flag) const
{
    return ( static_cast<u8_t>(m_allowed_connection) & static_cast<u8_t>(_flag) ) == static_cast<u8_t>(_flag);
}
