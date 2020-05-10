#include "Object.h"

using namespace Nodable;

Object::Object()
{
	add("__class__", OnlyWhenUncollapsed);
	add("name",      OnlyWhenUncollapsed);
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

bool Object::has(Member* _value)
{
	auto foundWithName = members.find(_value->getName());
	if( foundWithName != members.end())
		return (*foundWithName).second == _value;
	return false;
}

Member* Object::get (const char* _name)const
{
	auto foundWithName = members.find(std::string(_name));
	if (foundWithName != members.end())
		return (*foundWithName).second;
	return nullptr;
}

Member* Object::get (const std::string& _name)const
{
	return members.at(_name.c_str());
}

Member* Object::getFirstWithConn(Connection_ _connection)const
{
	Member* found = nullptr;

	auto m = this->members.begin();
	while (m != this->members.end() && found == nullptr)
	{
		if (m->second->getConnection() & _connection)
			found = m->second;
		m++;
	}

	return found;
}

void Object::add (const char* _name, Visibility_ _visibility, Type_ _type, Connection_ _flags )
{
	auto v = new Member();

	v->setOwner     (this);	
	v->setName		(_name);
	v->setVisibility(_visibility);
	v->setType		(_type);
	v->setConnectionFlags(_flags);
	members[std::string(_name)] = v;
}
