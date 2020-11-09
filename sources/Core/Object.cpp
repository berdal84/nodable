#include "Object.h"
#include "Member.h"

using namespace Nodable;

Object::Object()
{
	add("__class__", Visibility::OnlyWhenUncollapsed);
	add("name",      Visibility::OnlyWhenUncollapsed);
}

const std::map<std::string, std::shared_ptr<Member>>&   Object::getMembers      ()const
{
	return members;
}

bool Object::has(std::shared_ptr<Member> _value)
{
	auto foundWithName = members.find(_value->getName());
	if( foundWithName != members.end())
		return (*foundWithName).second == _value;
	return false;
}

std::shared_ptr<Member> Object::get (const char* _name)const
{
	auto foundWithName = members.find(std::string(_name));
	if (foundWithName != members.end())
		return (*foundWithName).second;
	return nullptr;
}

std::shared_ptr<Member> Object::get (const std::string& _name)const
{
	return members.at(_name);
}

std::shared_ptr<Member> Object::getFirstWithConn(Way _connection)const
{
	std::shared_ptr<Member> found;

	auto m = this->members.begin();
	while (m != this->members.end() && !found)
	{
		if (m->second->allowsConnections(_connection) )
			found = m->second;
		m++;
	}

	return found;
}

std::shared_ptr<Member> Object::add (const char* _name, Visibility _visibility, Type _type, Way _flags )
{
	auto member = std::make_shared<Member>();

	member->setOwner     (this);
	member->setName		(_name);
	member->setVisibility(_visibility);
	member->setType		(_type);
    member->setAllowedConnections(_flags);
	members.insert_or_assign(std::string(_name), member);

	return member;
}
