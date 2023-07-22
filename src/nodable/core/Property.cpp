#include "Property.h"

#include "PropertyGrp.h"
#include "VariableNode.h"

using namespace ndbl;

Property::Property(PropertyGrp * _group)
    : m_visibility(Visibility::Default)
    , m_name("Unknown")
    , token(Token::s_null)
    , m_input(nullptr)
    , m_property_group(_group)
    , m_allowed_connection(Way_Default)
    , m_variant()
    , m_is_reference(false)
{
    m_owner = _group ? _group->get_owner() : nullptr;
}

Property::Property(PropertyGrp * _group, double d ): Property(_group)
{ m_variant.set(d); }

Property::Property(PropertyGrp * _group, bool b): Property(_group)
{ m_variant.set(b); }

Property::Property(PropertyGrp * _group, int i): Property(_group, (double)i)
{}

Property::Property(PropertyGrp * _group, const char * str): Property(_group)
{ m_variant.set(str); }

Property::Property(PropertyGrp * _group, const std::string& s): Property(_group, s.c_str()){}


bool Property::has_input_connected() const
{
    return m_input != nullptr;
}

void Property::set_input(Property * _val)
{
    m_input        = _val;
}

void Property::digest(Property *_property)
{
    FW_EXPECT(_property->get_outputs().empty(), "Cannot digest a property with output connections")
    // Transfer
    m_variant.set(_property->m_variant);
    token = _property->token;
}

bool Property::is_connected_by_ref() const
{
    return m_input && is_reference();
}

void Property::ensure_is_defined(bool _value)
{
    deref_variant().flag_defined(_value);
}

bool Property::is_connected_to_variable() const
{
    return m_input && fw::extends<VariableNode>(m_input->get_owner());
}

VariableNode* Property::get_connected_variable()
{
    return fw::cast<VariableNode>(m_input->get_owner());
}

void Property::set(Node* _value)
{
    deref_variant().set(_value);
}

fw::qword* Property::get_underlying_data()
{
    return deref_variant().get_underlying_data();
}

std::shared_ptr<Property> Property::new_with_type(PropertyGrp *_parent, const fw::type* _type, Flags _flags)
{
    auto property = std::make_shared<Property>(_parent);
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

std::vector<fw::variant*>&Property::get_variant(std::vector<Property *> _in_properties, std::vector<fw::variant*>& _out_variants)
{
    for(Property* each_property : _in_properties)
    {
        _out_variants.push_back(each_property->get_variant());
    }
    return _out_variants;
}


bool Property::is_reference() const
{
    return m_is_reference || m_variant.get_type()->is_ptr();
}

void Property::set_reference(bool b)
{
    m_is_reference = b;
}