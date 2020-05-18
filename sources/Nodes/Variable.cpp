#include <Variable.h>
#include <Member.h>
#include <Log.h>

using namespace Nodable;

Variable::Variable()
{
	add("value", Always, Type_Unknown, Connection_InOut);	
}

Variable::~Variable()
{

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
	return getMember()->isType(_type);
}

std::string Variable::getTypeAsString()const
{
	return getMember()->getTypeAsString();
}
