#include <nodable/core/Node.h>
#include <nodable/core/PropertyGrp.h>

using namespace ndbl;

PropertyGrp::PropertyGrp(Node* _owner):m_owner(_owner){}

PropertyGrp::~PropertyGrp()
{
	for(Property * each_property : m_properties_by_id)
	{
        delete each_property;
	}
}

bool PropertyGrp::has(const char* _name)
{
    return m_properties_by_name.find(_name) != m_properties_by_name.end();
}

bool PropertyGrp::has(const Property * _property)
{
	return std::find(m_properties.cbegin(), m_properties.cend(), _property) != m_properties.end();
}

Property *PropertyGrp::add(const char* _name, Visibility _visibility, type _type, Way _way, Property::Flags _flags )
{
    NDBL_ASSERT(!has(_name));

    Property * new_property = Property::new_with_type(this, _type, _flags);
    new_property->set_name(_name);
    new_property->set_visibility(_visibility);
    new_property->set_allowed_connection(_way);

    add_to_indexes(new_property);

	return new_property;
}

Property *PropertyGrp::get_first(Way _way, type _type) const
{
    auto filter = [_way, _type](auto each_pair) -> bool
    {
        Property * each_property = each_pair.second;
        return type::is_implicitly_convertible(each_property->get_type(), _type)
               && ( each_property->get_allowed_connection() & _way );
    };

    auto found = std::find_if(m_properties_by_name.begin(), m_properties_by_name.end(), filter );

    if (found != m_properties_by_name.end() )
        return (*found).second;
    return nullptr;
}

void PropertyGrp::add_to_indexes(Property * _property)
{
    m_properties_by_name.insert({_property->get_name(), _property});
    m_properties_by_id.push_back(_property);
    m_properties.insert(_property);
}

Property *PropertyGrp::get_input_at(u8_t _position) const
{
    u8_t count = 0;

    for( auto each : m_properties_by_id)
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
