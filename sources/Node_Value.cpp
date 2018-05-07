#include "Node_Value.h"
#include "Log.h"		 // for LOG_DBG(...)

using namespace Nodable;

Node_Value::Node_Value(Type_ _type):
type(_type)
{
	LOG_DBG("New Node_Value\n");
}

Node_Value::~Node_Value(){};

Type_ Node_Value::getType()const
{
	return this->type;
}

bool  Node_Value::isType(Type_ _type)const
{
	return this->type == _type;
}
