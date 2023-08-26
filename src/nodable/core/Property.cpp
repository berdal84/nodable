#include "Property.h"

#include "PropertyBag.h"
#include "VariableNode.h"
#include "fw/core/Pool.h"

using namespace ndbl;

Property::Property()
    : m_visibility(Visibility::Default)
    , m_name("Unknown")
    , token(Token::s_null)
    , m_allowed_connection(Way_Default)
    , m_is_reference(false)
    , m_input(nullptr)
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


bool Property::has_input_connected() const
{
    return m_input != nullptr;
}

void Property::set_input(Property* _val)
{
    m_input = _val;
}

void Property::digest(Property* _property)
{
    FW_EXPECT(_property->get_outputs().empty(), "Cannot digest a property with output connections")
    // Transfer
    m_variant.set(_property->m_variant);
    token = std::move( _property->token );
}

bool Property::is_connected_by_ref() const
{
    return m_input != nullptr && is_reference();
}

void Property::ensure_is_defined(bool _value)
{
    value()->flag_defined(_value);
}

bool Property::is_connected_to_variable() const
{
    return m_input != nullptr && m_input->get_type()->is<VariableNode>();
}

VariableNode* Property::get_connected_variable()
{
    return m_input ? fw::cast<VariableNode>( m_input->owner().get() ) : nullptr;
}

void Property::set(Node* _value)
{
    value()->set(_value);
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


bool Property::is_reference() const
{
    return m_is_reference || m_variant.get_type()->is<ID<Node>>();
}

void Property::set_reference(bool b)
{
    m_is_reference = b;
}

void Property::ensure_is_initialized(bool b)
{
    value()->ensure_is_initialized(false);
}

bool Property::is_referencing_a_node() const
{
    return get_type()->is<ID<Node>>();
}

ID<Node> Property::value_as_node_id() const
{
    return (ID<Node>)*value();
}
