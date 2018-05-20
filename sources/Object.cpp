#include "Object.h"

using namespace Nodable;

const Members&   Object::getMembers      ()const
{
	return members;
}

Value* Object::getMember (const char* _name)const
{
	return members.at(std::string(_name));
}

Value* Object::getMember (const std::string& _name)const
{
	return members.at(_name.c_str());
}

void Object::addMember (const char* _name, Type_ _type)
{
	members[std::string(_name)] =  new Value(_type);
}