#include "Variable.h"
#include "Value.h"
#include "Log.h"

using namespace Nodable;

Variable::Variable()
{
	LOG_DBG("New Variable\n");
	addMember("value", Visibility_Public);
	setMember("class", "Variable");
}

Variable::~Variable()
{
}

double Variable::getValueAsNumber()const
{
	return getMember("value")->getValueAsNumber();
}

std::string Variable::getValueAsString()const
{
	return getMember("value")->getValueAsString();
}

void Variable::setName(const char* _name)
{
	name = _name;
	setLabel(_name);
}

const char* Variable::getName()const
{
	return name.c_str();
}

bool Variable::isType(Type_ _type)const
{
	return getValue()->isType(_type);
}

std::string Variable::getTypeAsString()const
{
	return getValue()->getTypeAsString();
}
