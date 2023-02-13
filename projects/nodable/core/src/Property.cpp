#include "fw/core/Log.h"
#include <ndbl/core/Property.h>
#include <ndbl/core/PropertyGrp.h>
#include <ndbl/core/VariableNode.h>

using namespace ndbl;

Property::Property(PropertyGrp * _parent_properties)
    : m_visibility(Visibility::Default)
    , m_name("Unknown")
    , m_sourceToken(Token::s_null)
    , m_input(nullptr)
    , m_parentProperties(_parent_properties)
    , m_allowed_connection(Way_Default)
    , m_variant()
{
    m_owner = _parent_properties ? _parent_properties->get_owner() : nullptr;
}

Property::Property(PropertyGrp * _parent_properties, double d ): Property(_parent_properties)
{
    m_variant.set(d);
}

Property::Property(PropertyGrp * _parent_properties, bool b): Property(_parent_properties)
{
    m_variant.set(b);
}

Property::Property(PropertyGrp * _parent_properties, int i): Property(_parent_properties, (double)i){}

Property::Property(PropertyGrp * _parent_properties, const char * str): Property(_parent_properties)
{
    m_variant.set(str);
}

Property::Property(PropertyGrp * _parent_properties, const std::string& s): Property(_parent_properties, s.c_str()){}

Property::~Property(){}

bool Property::has_input_connected() const
{
    return m_input != nullptr;
}

void Property::set_input(Property * _val)
{
    m_input        = _val;
}

void Property::set_src_token(const std::shared_ptr<Token> _token)
{
    if ( _token )
    {
        m_sourceToken = _token;
    }
    else
    {
        m_sourceToken = Token::s_null;
    }
}

void Property::digest(Property *_property)
{
    FW_EXPECT(_property->get_outputs().empty(), "Cannot digest a property with output connections")
    // Transfer
    m_variant     = _property->m_variant;
    m_sourceToken = _property->m_sourceToken;
}

bool Property::is_connected_by_ref() const
{
    const fw::type& variant_type = m_variant.get_type();
    return m_input && (variant_type.is_ptr() || variant_type.is_ref());
}

void Property::ensure_is_defined(bool _value)
{
    get_pointed_variant().flag_defined(_value);
}

bool Property::is_connected_to_variable() const
{
    return m_input && m_input->get_owner()->is<VariableNode>();
}

VariableNode*Property::get_connected_variable()
{
    return m_input->get_owner()->as<VariableNode>();
}

void Property::set(Node* _value)
{
    get_pointed_variant().set(_value);
}

fw::qword &Property::get_underlying_data()
{
    return get_pointed_variant().get_underlying_data();
}

std::shared_ptr<Property> Property::new_with_type(PropertyGrp *_parent, fw::type _type, Flags _flags)
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

std::vector<fw::variant*>&Property::get_variant(std::vector<Property *> _in, std::vector<fw::variant*>& _out)
{
    for(auto each : _in)
    {
        _out.push_back(each->get_variant());
    }
    return _out;
}
