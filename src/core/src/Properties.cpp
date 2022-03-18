#include <nodable/core/Properties.h>
#include <nodable/core/Node.h>

using namespace Nodable;

Properties::Properties(Node* _owner):m_owner(_owner){}

Properties::~Properties()
{
	for(auto each : m_props)
		delete each.second;
}

bool Properties::has(const char* _name)
{
    return m_props.find(_name) != m_props.end();
}

bool Properties::has(const Member* _value)
{
	auto found = m_props.find(_value->get_name());
	if(found != m_props.end())
		return (*found).second == _value;
	return false;
}

Member* Properties::add(const char* _name, Visibility _visibility, std::shared_ptr<const R::MetaType> _type, Way _flags )
{
	auto v = new Member(this);
    v->set_name(_name);
    v->set_visibility(_visibility);
    v->set_meta_type(_type);
    v->set_allowed_connection(_flags);
	m_props[std::string(_name)] = v;

	return v;
}

Member *Properties::get_first_member_with(Way _way, std::shared_ptr<const R::MetaType> _type) const
{
    auto filter = [_way, _type](auto each_pair) -> bool
    {
        Member* each_member = each_pair.second;
        return R::MetaType::is_convertible(each_member->get_meta_type(), _type)
               && ( each_member->get_allowed_connection() & _way );
    };

    auto found = std::find_if( m_props.begin(), m_props.end(), filter );

    if ( found != m_props.end() )
        return (*found).second;
    return nullptr;
}
