#include "Member.h"
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

bool Member::allows(Connection_ _connection)const
{
	auto maskedFlags = connection & _connection;
	return maskedFlags == _connection;
}

Object* Member::getOwner() const
{
	return owner;
}

Member* Member::getInput() const
{
	return input;
}

const std::string& Nodable::Member::getName() const
{
	return name;
}

void Member::setInput(Member* _val)
{
	input = _val;

	if (_val == nullptr)
		sourceExpression = "";
}

void Nodable::Member::setName(const char* _name)
{
	name = _name;
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
	if( input != nullptr)
		return input->getSourceExpression();

	if ( sourceExpression != "")
		return sourceExpression;

	return getValueAsString();
}