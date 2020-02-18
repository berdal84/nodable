#include "Member.h"
#include "Log.h"		 // for LOG_DBG(...)
#include "Object.h"
#include "Variable.h"

using namespace Nodable;

Member::Member()
{
	LOG_DBG("New Value\n");
}

Member::~Member(){};

Type_ Member::getType()const
{
	return data.getType();
}

bool  Member::isType(Type_ _type)const
{
	return data.isType(_type);
}

void Member::setConnectionFlags(Connection_ _flags)
{
	connection = _flags;
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
	this->setValue(this->inputMember);
}

bool Member::allows(Connection_ _connection)const
{
	auto maskedFlags = connection & _connection;
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

void Member::setValue(double _value)
{
	data.setType(Type_Number);
	data.setValue(_value);
	LOG_DBG("Value::setValue(%d)\n", _value);
}

void Member::setValue(std::string _value)
{
	this->setValue(_value.c_str());
}

void Member::setValue(const char* _value)
{
	data.setType(Type_String);
	data.setValue(_value);
	LOG_DBG("Value::setValue(%s)\n", _value);
}

void Member::setValue(bool _value)
{
	data.setType(Type_Boolean);
	data.setValue(_value);
	LOG_DBG("Value::setValue(%s)\n", _value ? "true" : "false");
}

double Member::getValueAsNumber()const
{
	return data.getValueAsNumber();
	
}

bool Member::getValueAsBoolean()const
{
	return data.getValueAsBoolean();	
}

std::string Member::getValueAsString()const
{
	return data.getValueAsString();
}

Visibility_ Member::getVisibility() const
{
	return visibility;
}

Connection_ Member::getConnection() const
{
	return connection;
}

bool Member::isSet()const
{
	return data.isSet();
}

void Nodable::Member::setOwner(Object* _owner)
{
	owner = _owner;
}

void Member::setValue(const Member* _v)
{
	data.setValue(&_v->data);	
}

std::string Member::getTypeAsString()const
{
	return data.getTypeAsString();
}

std::string Member::getSourceExpression()const
{
	std::string str;
	if ( allows(Connection_In) && inputMember != nullptr)
	{
		// if inputMember is a variable we add the variable name and an equal sign
		if (inputMember->getOwner()->getMember("__class__")->getValueAsString() == "Variable" &&
			getOwner()->getMember("__class__")->getValueAsString() == "Variable")
		{
			str.append(inputMember->getOwner()->getAs<Variable*>()->getName());
			str.append("=");
			str.append(inputMember->getSourceExpression());

		}else
			str = inputMember->getSourceExpression();

	} else if (sourceExpression != "")
		str = sourceExpression;
	else 
		str = getValueAsString();

	return str;
}