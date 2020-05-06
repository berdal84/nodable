#include "Member.h"
#include "Log.h"		 // for LOG_DEBUG(...)
#include "Object.h"
#include "Variable.h"

using namespace Nodable;

Member::Member()
{
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
}

void Member::setValue(std::string _value)
{
	this->setValue(_value.c_str());
}

void Member::setValue(const char* _value)
{
	data.setType(Type_String);
	data.setValue(_value);
}

void Member::setValue(bool _value)
{
	data.setType(Type_Boolean);
	data.setValue(_value);
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
	std::string expression;

	if ( allows(Connection_In) && inputMember != nullptr)
	{
		// if inputMember is a variable we add the variable name and an equal sign
		if (inputMember->getOwner()->get("__class__")->getValueAsString() == "Variable" &&
			getOwner()->get("__class__")->getValueAsString() == "Variable")
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
			expression = '"' + getValueAsString() + '"';
		}
		else {
			expression = getValueAsString();
		}
	}

	return expression;
}