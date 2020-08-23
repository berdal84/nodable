#include "Member.h"
#include "Log.h"		 // for LOG_DEBUG(...)
#include "Object.h"
#include "Variable.h"
#include "Language.h"

using namespace Nodable;

Member::Member()
{
}

Member::~Member(){
	if (in != nullptr)
		delete in;

	if (out != nullptr)
		delete out;
};

Type_ Member::getType()const
{
	return data.getType();
}

bool  Member::isType(Type_ _type)const
{
	return data.isType(_type);
}

bool Member::equals(const Member *_other)const {
	return _other != nullptr &&
	       _other->isType(this->getType() ) &&
		   (std::string)*_other == (std::string)*this;
}

void Member::setConnectionFlags(Connection_ _flags)
{
	// Delete existing (we could reuse...)
	if (in != nullptr)
		delete in;

	if (out != nullptr)
		delete out;

	// Create an input if needed
	if (_flags & Connection_In)
		in = new Connector(this, Connection_In);
	else
		in = nullptr;

	// Create an output if needed
	if (_flags & Connection_Out)
		out = new Connector(this, Connection_Out);
	else
		out = nullptr;
}

void Nodable::Member::setSourceExpression(const char* _val)
{
	sourceExpression = _val;
}

void Member::setType(Type_ _type)
{
	data.setType(_type);
}

void Member::setVisibility(Visibility_ _v)
{
	visibility = _v;
}

void Nodable::Member::updateValueFromInputMemberValue()
{
	this->set(this->inputMember);
}

bool Member::allows(Connection_ _connection)const
{
	auto maskedFlags = getConnectionFlags() & _connection;
	return maskedFlags == _connection;
}

Object* Member::getOwner() const
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

const TokenType_ Member::MemberTypeToTokenType(Type_ _type)
{
	if (_type == Type_Boolean) return TokenType_Boolean;
	if (_type == Type_Number)  return TokenType_Number;
	if (_type == Type_String)  return TokenType_String;

	return TokenType_Unknown;
}

const Type_ Nodable::Member::TokenTypeToMemberType(TokenType_ _tokenType)
{
	switch (_tokenType)
	{
	case TokenType_Boolean: return Type_Boolean;
	case TokenType_Number:  return Type_Number;
	case TokenType_String:  return Type_String;
	default:
		return Type_Unknown;
		break;
	}
	

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

Visibility_ Member::getVisibility() const
{
	return visibility;
}

Connection_ Member::getConnectionFlags() const
{
	if (in != nullptr && out != nullptr)
		return Connection_InOut;
	else if (out != nullptr)
		return Connection_Out;
	else if (in != nullptr)
		return Connection_In;
	else
		return Connection_None;
}

bool Member::isSet()const
{
	return data.isSet();
}

void Nodable::Member::setOwner(Object* _owner)
{
	owner = _owner;
}

std::string Member::getTypeAsString()const
{
	return data.getTypeAsString();
}

std::string Member::getSourceExpression()const
{
	std::string expression;

	if ( allows(Connection_In) && inputMember != nullptr)
	{
		// if inputMember is a variable we add the variable name and an equal sign
		if (inputMember->getOwner()->getClass()->getName() == "Variable" &&
			getOwner()->getClass()->getName() == "Variable")
		{
			auto variable = inputMember->getOwner()->as<Variable>();
			expression.append(variable->getName());
			expression.append("=");
			expression.append(inputMember->getSourceExpression());

		}else
			expression = inputMember->getSourceExpression();

	} else if (sourceExpression != "") {
		expression = sourceExpression;

	} else {

		if (isType(Type_String)) {
			expression = '"' + (std::string)*this + '"';
		}
		else {
			expression = (std::string)*this;
		}
	}

	return expression;
}


void Member::set(const Member* _v)
{
	data.set(&_v->data);
}

void Member::set(double _value)
{
	data.setType(Type_Number);
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
