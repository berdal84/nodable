#include "Member.h"
#include "Log.h"		 // for LOG_DEBUG(...)
#include "Properties.h"
#include "VariableNode.h"
#include "Language/Common/Language.h"

using namespace Nodable;

Member::Member()
    :
    owner(nullptr),
    sourceToken(Token::Null),
    out(nullptr),
    in(nullptr) {

}

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

Member::Member(std::string s): Member(s.c_str()){}

Member::~Member()
{
    delete in;
    delete out;
}

Type Member::getType()const
{
	return data.getType();
}

bool Member::hasInputConnected() const
{
    return this->getInputMember();
}

bool  Member::isType(Type _type)const
{
	return data.isType(_type) || getType() == Type_Any || _type == Type_Any;
}

bool Member::equals(const Member *_other)const {
	return _other != nullptr &&
	       _other->isType(this->getType() ) &&
		   (std::string)*_other == (std::string)*this;
}

void Member::setConnectorWay(Way _flags)
{
	// Delete existing (we could reuse...)
	if (in != nullptr)
		delete in;

	if (out != nullptr)
		delete out;

	// Create an input if needed
	if (_flags & Way_In)
		in = new Connector(this, Way_In);
	else
		in = nullptr;

	// Create an output if needed
	if (_flags & Way_Out)
		out = new Connector(this, Way_Out);
	else
		out = nullptr;
}

void Nodable::Member::setSourceExpression(const char* _val)
{
	sourceExpression = _val;
}

void Member::setType(Type _type)
{
	data.setType(_type);
}

void Member::setVisibility(Visibility _v)
{
	visibility = _v;
}

bool Member::allowsConnection(Way _way)const
{
	auto maskedFlags = getConnectorWay() & _way;
	return maskedFlags == _way;
}

Node* Member::getOwner() const
{
	return owner;
}

Member* Member::getInputMember() const
{
	return inputMember;
}

const std::string& Nodable::Member::getName() const
{
	return name;
}



const Nodable::Connector* Member::input() const
{
	return in;
}

const Nodable::Connector* Member::output() const
{
	return out;
}

void Member::setInputMember(Member* _val)
{
	inputMember = _val;

	if (_val == nullptr)
		sourceExpression = "";
}

void Nodable::Member::setName(const char* _name)
{
	name = _name;
}

Visibility Member::getVisibility() const
{
	return visibility;
}

Way Member::getConnectorWay() const
{
	if (in != nullptr && out != nullptr)
		return Way_InOut;
	else if (out != nullptr)
		return Way_Out;
	else if (in != nullptr)
		return Way_In;
	else
		return Way_None;
}

bool Member::isDefined()const
{
	return data.isSet();
}

std::string Member::getTypeAsString()const
{
	return data.getTypeAsString();
}

void Member::set(const Member* _v)
{
	data.set(&_v->data);
}

void Member::set(const Member& _v)
{
	data.set(&_v.data);
}

void Member::set(double _value)
{
	data.setType(Type_Double);
	data.set(_value);
}

void Member::set(int _value)
{
	set(double(_value));
}

void Member::set(const std::string& _value)
{
	this->set(_value.c_str());
}

void Member::set(const char* _value)
{
	data.setType(Type_String);
	data.set(_value);
}

void Member::set(bool _value)
{
	data.setType(Type_Boolean);
	data.set(_value);
}

void Member::setSourceToken(const Token* _token)
{
    if ( _token )
    {
        this->sourceToken = *_token;
    }
    else
    {
        this->sourceToken = Token::Null;
    }
}

void Member::digest(Member *_member)
{
    // Transfer
    this->data = _member->data;
    this->sourceToken = _member->sourceToken;

    // release member
    _member->sourceToken = Token::Null;
    delete _member;
}