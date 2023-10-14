#include "Property.h"

#include "PropertyBag.h"
#include "VariableNode.h"
#include "fw/core/Pool.h"

using namespace ndbl;

Property::Property()
    : m_name("Unknown")
    , token(Token::s_null)
    , m_flags( PropertyFlag_DEFAULT )
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


Property::Property(const fw::type* _type, PropertyFlags _flags)
: Property()
{
    // handle type
    m_variant.ensure_is_type(_type);

    // handle flags
    m_flags = _flags;
    if ( m_flags & PropertyFlag_INITIALIZE )  m_variant.ensure_is_initialized();
    if ( m_flags & PropertyFlag_DEFINE )      m_variant.flag_defined();
    if ( m_flags & PropertyFlag_RESET_VALUE ) m_variant.reset_value();
}


void Property::digest(Property* _property)
{
    // Transfer
    m_variant.set(_property->m_variant);
    token = std::move( _property->token );
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
    value()->ensure_is_initialized(b);
}

bool Property::is_ref() const
{
    return m_flags & PropertyFlag_IS_REF;
}

bool Property::is_type_null() const
{
    return get_type()->is<fw::null_t>();
}

bool Property::has_flags( PropertyFlags _flags ) const
{
    return (m_flags & _flags) == _flags;
}

void Property::flag_as_reference()
{
    m_flags = m_flags | PropertyFlag_IS_REF;
}

void Property::set_visibility(PropertyFlags _flags)
{
    auto new_visibility = _flags & PropertyFlag_VISIBILITY_MASK;
    auto flags_without_visibility = m_flags & ~PropertyFlag_VISIBILITY_MASK;
    m_flags = flags_without_visibility | new_visibility;
}

bool Property::is_this() const
{
    return id.m_value == 0;
}
