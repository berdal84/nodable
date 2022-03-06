#include <nodable/Properties.h>
#include <nodable/Node.h>

using namespace Nodable;

Properties::Properties(Node* _owner):m_owner(_owner){}

Properties::~Properties()
{
	for(auto each : m_props)
		delete each.second;
}

bool Properties::has(const std::string& _name)
{
    auto found = m_props.find(_name);
    if(found != m_props.end())
        return true;
    return false;
}

bool Properties::has(const Member* _value)
{
	auto found = m_props.find(_value->get_name());
	if(found != m_props.end())
		return (*found).second == _value;
	return false;
}

Member* Properties::get_first_member_with_conn(Way _connection)const
{
	auto filter = [_connection](auto each_pair) -> bool
    {
        return each_pair.second->get_allowed_connection() & _connection;
    };

	auto found = std::find_if( m_props.begin(), m_props.end(), filter );

	if ( found != m_props.end() )
        return (*found).second;
	return nullptr;
}

Member* Properties::add(const char* _name, Visibility _visibility, std::shared_ptr<const R::Type> _type, Way _flags )
{
	auto v = new Member(this);
    v->set_name(_name);
    v->set_visibility(_visibility);
    v->set_type(_type);
    v->set_allowed_connection(_flags);
	m_props[std::string(_name)] = v;

	return v;
}

Member *Properties::get_first_member_with(Way _way, std::shared_ptr<const R::Type> _type) const
{
    auto filter = [_way, _type](auto each_pair) -> bool
    {
        Member* each_member = each_pair.second;
        return      each_member->is_type(_type)
               && ( each_member->get_allowed_connection() & _way );
    };

    auto found = std::find_if( m_props.begin(), m_props.end(), filter );

    if ( found != m_props.end() )
        return (*found).second;
    return nullptr;
}
