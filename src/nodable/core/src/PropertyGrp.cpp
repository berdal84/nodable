#include <nodable/core/Node.h>
#include <nodable/core/PropertyGrp.h>

using namespace ndbl;

PropertyGrp::PropertyGrp(Node* _owner):m_owner(_owner){}

PropertyGrp::~PropertyGrp() {}

bool PropertyGrp::has(const char* _name) const
{
    return m_properties_.by_name.find(_name) != m_properties_.by_name.end();
}

std::shared_ptr<Property> PropertyGrp::add(const fw::type& _type, const char* _name, Visibility _visibility, Way _way, Property::Flags _flags )
{
    NDBL_ASSERT(!has(_name));

    // create the property
    auto new_property = Property::new_with_type(this, _type, _flags);
    new_property->set_name(_name);
    new_property->set_visibility(_visibility);
    new_property->set_allowed_connection(_way);

    // register property
    m_properties.insert(new_property);

    // add to indexes
    m_properties_.by_name.insert({new_property->get_name(), new_property.get()});
    m_properties_.by_index.push_back(new_property.get());

    return new_property;
}

Property *PropertyGrp::get_first(Way _way, const fw::type& _type) const
{
    auto filter = [_way, _type](auto each_pair) -> bool
    {
        Property * each_property = each_pair.second;
        return fw::type::is_implicitly_convertible(each_property->get_type(), _type)
               && ( each_property->get_allowed_connection() & _way );
    };

    auto found = std::find_if(m_properties_.by_name.begin(), m_properties_.by_name.end(), filter );

    return found != m_properties_.by_name.end() ? (*found).second : nullptr;
}

Property *PropertyGrp::get_input_at(u8_t _position) const
{
    u8_t count = 0;

    for( auto each : m_properties_.by_index)
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
