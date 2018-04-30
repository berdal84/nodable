#include "Node_Value.h"
#include "Node_String.h" // for dynamic cast
#include "Node_Number.h" // for dynamic cast
#include "Log.h"		 // for LOG_DBG(...)

using namespace Nodable;

Node_Value::Node_Value(Type_ _type):
type(_type)
{}

Node_Value::~Node_Value(){};

void  Node_Value::draw()
{
	printf("[%s]", asString()->getValue());
}

Type_ Node_Value::getType()const
{
	return this->type;
}

bool  Node_Value::isType(Type_ _type)const
{
	return this->type == _type;
}

 Node_Number*  Node_Value::asNumber()
{
	auto as = dynamic_cast<Node_Number*>(this);
	return as;
}

 Node_String*  Node_Value::asString()
{
	auto as = dynamic_cast<Node_String*>(this);
	return as;
}