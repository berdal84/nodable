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
	setValue(std::stod(_string));
}

double Node_Number::getValueAsNumber()const
{
	return this->value;
}

std::string Node_Number::getValueAsString()const
{
	return std::to_string(this->value);
}

void   Node_Number::setValue(double _value)
{
	this->value = _value;
	setLabel(std::to_string(_value));
}

void   Node_Number::setValue(const char* _value)
{
	setValue(std::stod(_value));
}