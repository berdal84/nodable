#include "Node_Variable.h"
#include "Value.h"
#include "Log.h"

using namespace Nodable;

Node_Variable::Node_Variable()
{
	LOG_DBG("New Node_Variable\n");
	addMember("value");
	setMember("class", "Node_Variable");
}

Node_Variable::~Node_Variable()
{
}

double Node_Variable::getValueAsNumber()const
{
	return getMember("value")->getValueAsNumber();
}

std::string Node_Variable::getValueAsString()const
{
	return getMember("value")->getValueAsString();
}

void Node_Variable::setName(const char* _name)
{
	name = _name;
	setLabel(_name);
}

const char* Node_Variable::getName()const
{
	return name.c_str();
}

bool Node_Variable::isType(Type_ _type)const
{
	return getValue()->isType(_type);
}

std::string Node_Variable::getTypeAsString()const
{
	return getValue()->getTypeAsString();
}
