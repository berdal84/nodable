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

void Node_String::setValue(const char* _value)
{
	LOG_MSG("Node_String : %s becomes %s\n", this->value.c_str(), _value);
	this->value = _value;
}

const char* Node_String::getValue()const
{
	return this->value.c_str();
}

bool Node_String::isEqualsTo(const Node_String* _right)const
{
	return value == _right->value;
}

bool Node_String::isEmpty()const
{
	return value == std::string("");
}