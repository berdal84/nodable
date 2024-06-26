#include "PropertyBag.h"
#include "Node.h"

using namespace ndbl;
using namespace tools;

bool PropertyBag::has(const char* _name) const
{
    return m_properties_by_name.find(_name) != m_properties_by_name.end();
}

ID<Property> PropertyBag::add(const type* _type, const char* _name, PropertyFlags _flags )
{
    ASSERT(!has(_name));

    auto id = (ID<Property>)m_properties.size(); // we cannot delete a property, so we can count on size() to get a unique id

    // create the property
    Property& new_property = m_properties.emplace_back(_type, _flags);
    new_property.set_name(_name);
    new_property.id = id;

    // Index by name
    m_properties_by_name.insert({new_property.get_name(), new_property.id});

    return new_property.id;
}

const Property* PropertyBag::find_first( PropertyFlags _flags, const type *_type) const
{
    return _find_first( _flags, _type );
}

Property* PropertyBag::find_first( PropertyFlags _flags, const type *_type)
{
    return const_cast<Property*>( _find_first( _flags, _type ) );
}

const Property* PropertyBag::_find_first( PropertyFlags _flags, const type *_type) const
{
    auto filter = [this, _flags, _type](const std::pair<const std::string, ID<Property>>& pair) -> bool
    {
        auto& property = m_properties[(u32_t)pair.second];
        return type::is_implicitly_convertible( property.get_type(), _type)
               && ( property.has_flags( _flags ) );
    };

    auto found = std::find_if(m_properties_by_name.begin(), m_properties_by_name.end(), filter );
    if ( found != m_properties_by_name.end())
        return at(found->second);
    return nullptr;
}

Property* PropertyBag::find_by_name(const char *_name)
{
    ID<Property> id = m_properties_by_name.at(_name);
    return &m_properties[(u32_t)id];
}

const Property* PropertyBag::find_by_name(const char *_name) const
{
    auto it = m_properties_by_name.find(_name);
    if( it != m_properties_by_name.end())
    {
        return &m_properties[it->second.id()];
    }
    return nullptr;
}

Property* PropertyBag::at(ID<Property> property)
{
    return &m_properties[property.id()];
}

const Property* PropertyBag::at(ID<Property> property) const
{
    return &m_properties[property.id()];
}

ID<Property> PropertyBag::find_id_from_name(const char *_name) const
{
    for(auto& [name, id] : m_properties_by_name )
    {
        if( name == _name)
        {
            return id;
        }
    }
    EXPECT( false, "No property found with this name")
}

Property* PropertyBag::get_this()
{
    return &m_properties[THIS_ID];
}

const Property* PropertyBag::get_this() const
{
    return &m_properties[THIS_ID];
}

size_t PropertyBag::size() const
{
    return m_properties.size();
}
