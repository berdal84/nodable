#include "Node_Number.h"
#include "Log.h"		// for LOG_DBG(...)

using namespace Nodable;

Node_Number::~Node_Number(){}

Node_Number::Node_Number(double _n):
	Node_Value(Type_Number), 
	value(_n)
{
}

Node_Number::Node_Number(const char* _string):
	Node_Value(Type_Number)
{
	value = std::stod(_string);
}

void  Node_Number::draw()
{
	printf("[%s]", getValueAsString().c_str());
}

double Node_Number::getValueAsNumber()const
{
	return this->value;
}

std::string Node_Number::getValueAsString()const
{
	return std::to_string(this->value);
}

std::string Node_Number::getLabel()const
{
	return getValueAsString();
}

void   Node_Number::setValue(double _value)
{
	this->value = _value;
}

void   Node_Number::setValue(const char* _value)
{
	this->value = std::stod(_value);
}