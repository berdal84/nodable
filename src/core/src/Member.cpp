#include <nodable/core/Member.h>
#include <nodable/core/Log.h> // for LOG_DEBUG(...)
#include <nodable/core/Properties.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/Language.h>

using namespace Nodable;

Member::Member(Properties* _parent_properties)
    : m_visibility(Visibility::Default)
    , m_name("Unknown")
    , m_sourceToken(Token::s_null)
    , m_input(nullptr)
    , m_parentProperties(_parent_properties)
    , m_connected_by(ConnectBy_Copy)
    , m_allowed_connection(Way_Default)
    , m_variant()
{
    m_owner = _parent_properties ? _parent_properties->get_owner() : nullptr;
}

Member::Member( Properties* _parent_properties, double d ): Member(_parent_properties)
{
    m_variant.define_type<double>();
    m_variant.set(d);
}

Member::Member(Properties* _parent_properties, bool b): Member(_parent_properties)
{
    m_variant.define_type<bool>();
    m_variant.set(b);
}

Member::Member(Properties* _parent_properties, int i): Member(_parent_properties, (double)i){}

Member::Member(Properties* _parent_properties, const char * str): Member(_parent_properties)
{
    m_variant.define_type<std::string>();
    m_variant.set(str);
}

Member::Member(Properties* _parent_properties, const std::string& s): Member(_parent_properties, s.c_str()){}

Member::~Member(){}

bool Member::has_input_connected() const
{
    return m_input != nullptr;
}

void Member::set_input(Member* _val)
{
    m_input        = _val;
    type type = m_variant.get_type();
    m_connected_by = type.is_ref() || type.is_ptr() ? ConnectBy_Ref : ConnectBy_Copy;
}

void Member::set(Node* _value)
{
    get_variant().set(_value);
}

void Member::set(double _value)
{
    get_variant().set(_value);
}

void Member::set(const char* _value)
{
    get_variant().set(_value);
}

void Member::set(const std::string& _value)
{
    get_variant().set(_value);
}

void Member::set(bool _value)
{
    get_variant().set(_value);
}

void Member::set(i16_t _value)
{
    get_variant().set(_value);
}

void Member::set_src_token(const std::shared_ptr<Token> _token)
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

void Member::digest(Member *_member)
{
    // Transfer
    m_variant.set( _member->m_variant );
    m_sourceToken = _member->m_sourceToken;
}

bool Member::is_connected_by(ConnBy_ by)
{
    return m_connected_by == by;
}

void Member::force_defined_flag(bool _value)
{
    get_variant().force_defined_flag(_value);
}

assembly::QWord* Member::get_data_ptr()
{
    return m_variant.get_data_ptr();
}

bool Member::is_connected_to_variable() const
{
    return m_input && m_input->get_owner()->is<VariableNode>();
}

VariableNode* Member::get_connected_variable()
{
    return m_input->get_owner()->as<VariableNode>();
}
