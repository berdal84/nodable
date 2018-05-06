#include "Node_Number.h"
#include "Log.h"		// for LOG_DBG(...)

using namespace Nodable;

Node_Number::~Node_Number(){}

Node_Number::Node_Number(double _n):
	Node_Value(Type_Number)
{
	setValue(_n);
}

Node_Number::Node_Number(const char* _string):
	Node_Value(Type_Number)
{
	setValue(_string);
}

double Node_Number::getValueAsNumber()const
{
	return this->value;
}

std::string Node_Number::getValueAsString()const
{
	// Format the num as a string without any useless ending zeros/dot
	std::string str = std::to_string (this->value);
	str.erase ( str.find_last_not_of('0') +1, std::string::npos );
	if (str.find_last_of('.') +1 == str.size())
		str.erase ( str.find_last_of('.'), std::string::npos );
	return str;
}

void   Node_Number::setValue(double _value)
{
	this->value = _value;
	setLabel(getValueAsString());
}

void   Node_Number::setValue(const char* _value)
{
	setValue(std::stod(_value));
}