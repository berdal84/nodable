#include "Value.h"
#include "Log.h"		 // for LOG_DBG(...)

using namespace Nodable;

Value::Value()
{
	LOG_DBG("New Value\n");
}

Value::~Value(){};

Type_ Value::getType()const
{
	return data.getType();
}

bool  Value::isType(Type_ _type)const
{
	return data.isType(_type);
}

void Value::setConnectionFlags(ConnectionFlags_ _flags)
{
	connectionFlags = _flags;
}

bool Value::allows(ConnectionFlags_ _flags)const
{
	auto maskedFlags = connectionFlags & _flags;
	return maskedFlags == _flags;
}

void Value::setInput(Value* _val)
{
	input = _val;

	if (_val == nullptr)
		sourceExpression = "";
}

void Value::setValue(double _value)
{
	data.setValue(_value);
	LOG_DBG("Value::setValue(%d)\n", _value);
}

void Value::setValue(std::string _value)
{
	data.setValue(_value.c_str());
}

void Value::setValue(const char* _value)
{
	data.setValue(_value);
	LOG_DBG("Value::setValue(%s)\n", _value);
}

void Value::setValue(bool _value)
{
	data.setValue(_value);
	LOG_DBG("Value::setValue(%s)\n", _value ? "true" : "false");
}

double Value::getValueAsNumber()const
{
	return data.getValueAsNumber();
	
}

bool Value::getValueAsBoolean()const
{
	return data.getValueAsBoolean();	
}

std::string Value::getValueAsString()const
{
	return data.getValueAsString();
}

bool Value::isSet()const
{
	return data.isSet();
}

void Value::setValue(const Value* _v)
{
	data = _v->data;	
}

std::string Value::getTypeAsString()const
{
	return data.getTypeAsString();
}

std::string Value::getSourceExpression()const
{
	if( input != nullptr)
		return input->getSourceExpression();

	if ( sourceExpression != "")
		return sourceExpression;

	return getValueAsString();
}