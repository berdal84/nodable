#include <nodable/Member.h>
#include <nodable/Log.h> // for LOG_DEBUG(...)
#include <nodable/Properties.h>
#include <nodable/VariableNode.h>
#include <nodable/Language.h>

using namespace Nodable;

Member::Member(Properties* _parent_properties)
    : m_visibility(Visibility::Default)
    , m_name("Unknown")
    , m_sourceToken(Token::s_null)
    , m_input(nullptr)
    , m_parentProperties(_parent_properties)
    , m_connected_by(ConnectBy_Copy)
{
    m_owner = _parent_properties ? _parent_properties->get_owner() : nullptr;
}

Member::Member( Properties* _parent_properties, double d ): Member(_parent_properties)
{
    m_variant.set_meta_type<double>();
    m_variant.set(d);
}

Member::Member(Properties* _parent_properties, bool b): Member(_parent_properties)
{
    m_variant.set_meta_type<bool>();
    m_variant.set(b);
}

Member::Member(Properties* _parent_properties, int i): Member(_parent_properties, (double)i){}

Member::Member(Properties* _parent_properties, const char * str): Member(_parent_properties)
{
    m_variant.set_meta_type<std::string>();
    m_variant.set(str);
}

Member::Member(Properties* _parent_properties, const std::string& s): Member(_parent_properties, s.c_str()){}

Member::Member(Properties* _parent_properties, Node* _node): Member(_parent_properties)
{
    m_variant.set(_node);
}

Member::~Member(){}

bool Member::has_input_connected() const
{
    return m_input != nullptr;
}

bool Member::equals(const Member *_other)const {
	return _other != nullptr &&
            _other->m_variant.get_meta_type() == m_input->m_variant.get_meta_type() &&
		   (std::string)*_other == (std::string)*m_input;
}

void Member::set_input(Member* _val, ConnBy_ _connect_by)
{
    m_input = _val;
    m_connected_by = _connect_by;
}

void Member::set(Node* _value)
{
    get_variant().set_meta_type(R::get_meta_type<Node *>());
    get_variant().set(_value);
}

void Member::set(double _value)
{
    get_variant().set_meta_type(R::get_meta_type<double>());
    get_variant().set(_value);
}

void Member::set(const char* _value)
{
    get_variant().set_meta_type(R::get_meta_type<std::string>());
    get_variant().set(_value);
}

void Member::set(bool _value)
{
    get_variant().set_meta_type(R::get_meta_type<bool>());
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
    m_variant = _member->m_variant;
    m_sourceToken = _member->m_sourceToken;

    // release member
    _member->m_sourceToken = Token::s_null;
}

bool Member::is_connected_by(ConnBy_ by) {
    return m_connected_by == by;
}

void Member::define()
{
    m_variant.define();
}
