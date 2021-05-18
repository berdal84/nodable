#include <nodable/Properties.h>
#include <nodable/Node.h>

using namespace Nodable;

Properties::Properties(Node* _owner):m_owner(_owner){}

Properties::~Properties()
{
	for(auto each : m_props)
		delete each.second;
}


const Members&   Properties::getMembers      ()const
{
	return m_props;
}

bool Properties::has(const Member* _value)
{
	auto foundWithName = m_props.find(_value->getName());
	if(foundWithName != m_props.end())
		return (*foundWithName).second == _value;
	return false;
}

Member* Properties::get (const char* _name)const
{
	auto foundWithName = m_props.find(std::string(_name));
	if (foundWithName != m_props.end())
		return (*foundWithName).second;
	return nullptr;
}

Member* Properties::get (const std::string& _name)const
{
	return m_props.at(_name.c_str());
}

Member* Properties::getFirstWithConn(Way _connection)const
{
	Member* found = nullptr;

	auto m = this->m_props.begin();
	while (m != this->m_props.end() && found == nullptr)
	{
		if (m->second->getConnectorWay() & _connection)
			found = m->second;
		m++;
	}

	return found;
}

Member* Properties::add (const char* _name, Visibility _visibility, Type _type, Way _flags )
{
	auto v = new Member();
	v->setParentProperties(this);
	v->setOwner(this->m_owner);
	v->setName		(_name);
	v->setVisibility(_visibility);
	v->setType		(_type);
	v->setConnectorWay(_flags);
	m_props[std::string(_name)] = v;

	return v;
}
