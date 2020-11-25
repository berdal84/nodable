#include "Object.h"

using namespace Nodable;

Object::Object()
{
	add("__class__", Visibility::OnlyWhenUncollapsed);
	add("name",      Visibility::OnlyWhenUncollapsed);
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

bool Object::has(const Member* _value)
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

Member* Object::getFirstWithConn(Way _connection)const
{
	Member* found = nullptr;

	auto m = this->members.begin();
	while (m != this->members.end() && found == nullptr)
	{
		if (m->second->getConnectorWay() & _connection)
			found = m->second;
		m++;
	}

	return found;
}

Member* Object::add (const char* _name, Visibility _visibility, Type _type, Way _flags )
{
	auto v = new Member(this);
	v->setName		(_name);
	v->setVisibility(_visibility);
	v->setType		(_type);
	v->setConnectorWay(_flags);
	members[std::string(_name)] = v;

	return v;
}
