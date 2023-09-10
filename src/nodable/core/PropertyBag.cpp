#include "PropertyBag.h"
#include "Node.h"

using namespace ndbl;
using fw::ID;

PropertyBag::~PropertyBag()
{
    // Properties are stored in m_properties
}

bool PropertyBag::has(const char* _name) const
{
    return m_properties_by_name.find(_name) != m_properties_by_name.end();
}

ID<Property> PropertyBag::add(const fw::type* _type, const char* _name, Visibility _visibility, Way _way, Property::Flags _flags )
{
    FW_ASSERT(!has(_name));

    // create the property
    Property* new_property = Property::new_with_type(_type, _flags);
    new_property->set_name(_name);
    new_property->set_visibility(_visibility);
    new_property->set_allowed_connection(_way);
    new_property->id = (ID<Property>)m_properties.size(); // we cannot delete a property, so we can count on size() to get a unique id

    // Index by name
    m_properties_by_name.insert({new_property->get_name(), new_property->id});

    // register property
    m_properties.emplace_back( std::move(new_property) );

    return new_property->id;
}

const Slot* PropertyBag::get_first(Way _way, const fw::type *_type) const
{
    return get_first(_way,_type);
}

Slot* PropertyBag::get_first(Way _way, const fw::type *_type)
{
    auto filter = [this, _way, _type](std::pair<const std::string, ID<Property>>& each) -> bool
    {
        auto& [_, pos] = each;
        auto& property = m_properties[(size_t)pos];
        return fw::type::is_implicitly_convertible( property.get_type(), _type)
               && ( property.allows_connection(_way) );
    };

    auto found = std::find_if(m_properties_by_name.begin(), m_properties_by_name.end(), filter );
    if ( found != m_properties_by_name.end())
        return at(found->second);
    return nullptr;
}

const Property* PropertyBag::get_input_at(ID<Property> _position) const
{
    return get_input_at(_position);
}

Property* PropertyBag::get_input_at(ID<Property> _position)
{
    size_t count = 0;
    for(auto& each_property : m_properties)
    {
        if( each_property.allows_connection(Way::In) && !each_property.allows_connection(Way::Out))
        {
            if( count == (size_t)_position)
            {
                return &each_property;
            }
            count++;
        }
    }
    return nullptr;
}

PropertyBag::PropertyBag(PropertyBag&& other)
{
    *this = std::move(other);
}

PropertyBag& PropertyBag::operator=(PropertyBag&& other )
{
    m_properties_by_name = std::move(other.m_properties_by_name);
    m_properties  = std::move(other.m_properties);
    return *this;
}

Property* PropertyBag::get(const char *_name)
{
    ID<Property> id = m_properties_by_name.at(_name);
    return &m_properties[(size_t)id];
}

const Property* PropertyBag::get(const char *_name) const
{
    ID<Property> id = m_properties_by_name.at(_name);
    return &m_properties[(size_t)id];
}

Property* PropertyBag::at(ID<Property> property)
{
    return &m_properties[property.id()];
}

const Property* PropertyBag::at(ID<Property> property) const
{
    return &m_properties[property.id()];
}

ID<Property> PropertyBag::get_id(const char *_name) const
{
    for(auto& [name, id] : m_properties_by_name )
    {
        if( name == _name)
        {
            return id;
        }
    }
    FW_EXPECT( false, "No property found with this name")
}

Property* PropertyBag::get_this()
{
    return &m_properties[THIS_ID];
}

const Property* PropertyBag::get_this() const
{
    return &m_properties[THIS_ID];
}
