#include "Member.h"
#include "Log.h"		 // for LOG_DEBUG(...)
#include "Properties.h"
#include "VariableNode.h"
#include "Language/Common/Language.h"

using namespace Nodable;

Member::Member()
    :
    m_owner(nullptr),
    m_visibility(Visibility::Default),
    m_name("Unknown"),
    m_sourceToken(Token::s_null),
    m_inputMember(nullptr),
    m_parentProperties(nullptr)
    {}

Member::Member(double d): Member()
{
    set(d);
}

Member::Member(bool b): Member()
{
    set(b);
}

Member::Member(int i): Member((double)i){}

Member::Member(const char * str): Member() {
    set(str);
}

Member::Member(const std::string& s): Member(s.c_str()){}

Member::~Member(){}

bool Member::hasInputConnected() const
{
    return this->getInputMember();
}

bool Member::equals(const Member *_other)const {
	return _other != nullptr &&
	       _other->m_data.getType() == this->m_data.getType() &&
		   (std::string)*_other == (std::string)*this;
}

void Member::setInputMember(Member* _val)
{
    m_inputMember = _val;

	if (_val == nullptr)
        m_sourceExpression = "";
}

void Member::set(double _value)
{
	m_data.setType(Type_Double);
	m_data.set(_value);
}

void Member::set(const char* _value)
{
	m_data.setType(Type_String);
	m_data.set(_value);
}

void Member::set(bool _value)
{
	m_data.setType(Type_Boolean);
	m_data.set(_value);
}

void Member::setSourceToken(const Token* _token)
{
    if ( _token )
    {
        this->m_sourceToken = *_token;
    }
    else
    {
        this->m_sourceToken = Token::s_null;
    }
}

void Member::digest(Member *_member)
{
    // Transfer
    this->m_data = _member->m_data;
    this->m_sourceToken = _member->m_sourceToken;

    // release member
    _member->m_sourceToken = Token::s_null;
}