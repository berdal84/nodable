#include <nodable/core/Member.h>
#include <nodable/core/Log.h> // for LOG_DEBUG(...)
#include <nodable/core/Properties.h>
#include <nodable/core/VariableNode.h>

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
    m_variant.set(d);
}

Member::Member(Properties* _parent_properties, bool b): Member(_parent_properties)
{
    m_variant.set(b);
}

Member::Member(Properties* _parent_properties, int i): Member(_parent_properties, (double)i){}

Member::Member(Properties* _parent_properties, const char * str): Member(_parent_properties)
{
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
    NODABLE_ASSERT_EX(_member->get_outputs().empty(), "Cannot digest a member with output connections")
    // Transfer
    m_variant     = _member->m_variant;
    m_sourceToken = _member->m_sourceToken;
}

bool Member::is_connected_by_ref()
{
    const type& variant_type = m_variant.get_type();
    return m_input && (variant_type.is_ptr() || variant_type.is_ref());
}

void Member::ensure_is_defined(bool _value)
{
    get_pointed_variant().flag_defined(_value);
}

bool Member::is_connected_to_variable() const
{
    return m_input && m_input->get_owner()->is<VariableNode>();
}

VariableNode* Member::get_connected_variable()
{
    return m_input->get_owner()->as<VariableNode>();
}

void Member::set(Node* _value)
{
    get_pointed_variant().set(_value);
}

qword& Member::get_underlying_data()
{
    return get_pointed_variant().get_underlying_data();
}

Member* Member::new_with_type(Properties *_parent, type _type, Flags _flags)
{
    auto member = new Member(_parent);
    member->m_variant.ensure_is_type(_type);

    if( _flags & Flags_initialize )
    {
        member->m_variant.ensure_is_initialized();
    }

    if(_flags & Flags_define )
    {
        member->m_variant.flag_defined();
    }

    if(_flags & Flags_reset_value )
    {
        member->m_variant.reset_value();
    }

    return member;
}

std::vector<variant*>& Member::get_variant(std::vector<Member *> _in, std::vector<variant*>& _out)
{
    for(auto each : _in)
    {
        _out.push_back(each->get_variant());
    }
    return _out;
}
