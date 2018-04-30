#include "Node_Number.h"
#include "Log.h"		// for LOG_DBG(...)

using namespace Nodable;

Node_Number::~Node_Number(){}

Node_Number::Node_Number(double _n):
	Node_Value(Type_Number), 
	value(_n)
{}

void  Node_Number::draw()
{
	printf("[%f]", getValue());
}

Node_Number::Node_Number(std::string _string):
	Node_Value(Type_Number),
	value(std::stod(_string))
{

}

double Node_Number::getValue()const
{
	return this->value;
}

void   Node_Number::setValue(double _value)
{
	this->value = _value;
}