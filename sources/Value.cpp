#include "Value.h"
#include "Log.h"		 // for LOG_DBG(...)

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

void Member::setConnectionFlags(ConnectionFlags_ _flags)
{
	connectionFlags = _flags;
}

bool Member::allows(ConnectionFlags_ _flags)const
{
	auto maskedFlags = connectionFlags & _flags;
	return maskedFlags == _flags;
}

void Member::setInput(Member* _val)
{
	input = _val;

	if (_val == nullptr)
		sourceExpression = "";
}

void Member::setValue(double _value)
{
	data.setValue(_value);
	LOG_DBG("Value::setValue(%d)\n", _value);
}

void Member::setValue(std::string _value)
{
	data.setValue(_value.c_str());
}

void Member::setValue(const char* _value)
{
	data.setValue(_value);
	LOG_DBG("Value::setValue(%s)\n", _value);
}

void Member::setValue(bool _value)
{
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

bool Member::isSet()const
{
	return data.isSet();
}

void Member::setValue(const Member* _v)
{
	data = _v->data;	
}

std::string Member::getTypeAsString()const
{
	return data.getTypeAsString();
}

std::string Member::getSourceExpression()const
{
	if( input != nullptr)
		return input->getSourceExpression();

	if ( sourceExpression != "")
		return sourceExpression;

	return getValueAsString();
}