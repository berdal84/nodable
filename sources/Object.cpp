#include "Object.h"

using namespace Nodable;

Object::Object()
{
	addMember("__class__", Visibility_Protected);
	addMember("name",      Visibility_Protected);
}

Object::~Object()
{
	for(auto each : members)
		delete each.second;
}


const Members&   Object::getMembers      ()const
{
	return members;
}

bool Object::hasMember(Member* _value)
{
	auto foundWithName = members.find(_value->getName());
	if( foundWithName != members.end())
		return (*foundWithName).second == _value;
	return false;

}

Member* Object::getMember (const char* _name)const
{
	return members.at(std::string(_name));
}

Member* Object::getMember (const std::string& _name)const
{
	return members.at(_name.c_str());
}

void Object::addMember (const char* _name, Visibility_ _visibility, Type_ _type, ConnectionFlags_ _flags )
{
	auto v = new Member();

	v->setOwner     (this);	
	v->setName		(_name);
	v->setVisibility(_visibility);
	v->setType		(_type);
	v->setConnectionFlags(_flags);
	members[std::string(_name)] = v;
}