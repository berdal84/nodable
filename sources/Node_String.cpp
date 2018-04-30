#include "Node_String.h"
#include "Log.h"		// for LOG_DBG(...)

using namespace Nodable;

Node_String::Node_String(const char* _value):
	Node_Value(Type_String),
	value(_value)
{
	LOG_DBG("New Node_String : %s", _value);
}

Node_String::~Node_String(){}

void Node_String::setValue(const char* _value)
{
	LOG_MSG("Node_String : %s becomes %s", this->value.c_str(), _value);
	this->value = _value;
}

const char* Node_String::getValue()const
{
	return this->value.c_str();
}