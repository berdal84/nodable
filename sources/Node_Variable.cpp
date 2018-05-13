#include "Node_Variable.h"
#include "Value.h"
#include "Log.h"

using namespace Nodable;

Node_Variable::Node_Variable()
{
	LOG_DBG("New Node_Variable\n");
	addMember("value");
}

Node_Variable::~Node_Variable()
{
}

bool Node_Variable::eval()
{
	updateLabel();
	return true;
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
	updateLabel();
}

void Node_Variable::updateLabel()
{
	if ( name != "")
		setLabel(getName() + std::string(" : ") + getMember("value")->getValueAsString());
	else
		setLabel(getMember("value")->getValueAsString());
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
	if (isSet())
		return getValue()->getTypeAsString();
	return "Unknown";
}
