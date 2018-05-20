#include "Object.h"

using namespace Nodable;

Object::Object()
{
	addMember("class");
	getMember("class")->setVisibility(Visibility_Protected);
}

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

void Object::addMember (const char* _name, Visibility_ _visibility, Type_ _type)
{
	auto v = new Value();
	
	v->setName		(_name);
	v->setVisibility(_visibility);
	v->setType		(_type);

	members[std::string(_name)] = v;
}