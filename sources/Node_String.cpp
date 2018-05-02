#include "Node_String.h"
#include "Log.h"		// for LOG_DBG(...)

using namespace Nodable;

Node_String::Node_String(const char* _value):
	Node_Value(Type_String),
	value(_value)
{
	LOG_DBG("New Node_String : %s\n", _value);
}

Node_String::~Node_String(){}

bool Node_String::isEqualsTo(const Node_String* _right)const
{
	return value == _right->value;
}

bool Node_String::isEmpty()const
{
	return value == std::string("");
}

double Node_String::getValueAsNumber()const
{
	return double{this->value.size()};
}

std::string Node_String::getValueAsString()const
{
	return this->value;
}

std::string Node_String::getLabel()const
{
	return this->value;
}

void Node_String::setValue(const char* _value)
{
	LOG_MSG("Node_String : %s becomes %s\n", this->value.c_str(), _value);
	this->value = _value;
}

void Node_String::setValue(double _value)
{
	this->setValue(std::to_string(_value).c_str());
}