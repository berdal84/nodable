#include <nodable/core/Properties.h>
#include <nodable/core/Node.h>

using namespace Nodable;

Properties::Properties(Node* _owner):m_owner(_owner){}

Properties::~Properties()
{
	for(Member* each_member : m_members_by_id)
	{
        delete each_member;
	}
}

bool Properties::has(const char* _name)
{
    return m_members_by_name.find(_name) != m_members_by_name.end();
}

bool Properties::has(const Member* _member)
{
	return std::find(m_members.cbegin(), m_members.cend(), _member) != m_members.end();
}

Member* Properties::add(const char* _name, Visibility _visibility, type _type, Way _flags )
{
    NODABLE_ASSERT(_type != type::null );
    NODABLE_ASSERT(!has(_name));

	Member* new_member = Member::new_with_type(this, _type);
    new_member->set_name(_name);
    new_member->set_visibility(_visibility);
    new_member->set_allowed_connection(_flags);

    add_to_indexes(new_member);

	return new_member;
}

Member *Properties::get_first(Way _way, type _type) const
{
    auto filter = [_way, _type](auto each_pair) -> bool
    {
        Member* each_member = each_pair.second;
        return type::is_implicitly_convertible(each_member->get_type(), _type)
               && ( each_member->get_allowed_connection() & _way );
    };

    auto found = std::find_if(m_members_by_name.begin(), m_members_by_name.end(), filter );

    if (found != m_members_by_name.end() )
        return (*found).second;
    return nullptr;
}

void Properties::add_to_indexes(Member* _member)
{
    m_members_by_name.insert({_member->get_name(), _member});
    m_members_by_id.push_back(_member);
    m_members.insert(_member);
}

Member* Properties::get_input_at(u8_t _position) const
{
    u8_t count = 0;

    for( auto each : m_members_by_id)
    {
        if( each->allows_connection(Way_In) && !each->allows_connection(Way_Out))
        {
            if( count == _position)
            {
                return each;
            }
            count++;
        }
    }
    return nullptr;
}
