#include "Node_Value.h"
#include "Node_String.h" // for dynamic cast
#include "Node_Number.h" // for dynamic cast
#include "Log.h"		 // for LOG_DBG(...)

using namespace Nodable;

Node_Value::Node_Value(Type_ _type):
type(_type)
{
	LOG_DBG("New Node_Value\n");
}

Node_Value::~Node_Value(){};

void  Node_Value::draw()
{
	printf("[%s]", getValueAsString().c_str());
}

Type_ Node_Value::getType()const
{
	return this->type;
}

bool  Node_Value::isType(Type_ _type)const
{
	return this->type == _type;
}
