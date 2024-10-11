#include "PropertyBag.h"
#include "Node.h"

using namespace ndbl;
using namespace tools;

bool PropertyBag::has(const char* _name) const
{
    return m_properties_by_name.find(_name) != m_properties_by_name.end();
}

Property* PropertyBag::add(const TypeDesc* _type, const char* _name, PropertyFlags _flags )
{
    VERIFY(m_owner != nullptr, "PropertyBag must be initialized")
    ASSERT(!has(_name));

    // create the property

    auto* new_property = new Property();
    new_property->init(_type, _flags, m_owner, _name);

    m_properties.push_back(new_property);
    // Index by name
    m_properties_by_name.insert({_name, new_property});

    return new_property;
}

const Property* PropertyBag::find_first( PropertyFlags _flags, const TypeDesc *_type) const
{
    return _find_first( _flags, _type );
}

Property* PropertyBag::find_first( PropertyFlags _flags, const TypeDesc *_type)
{
    return const_cast<Property*>( _find_first( _flags, _type ) );
}

const Property* PropertyBag::_find_first( PropertyFlags _flags, const TypeDesc *_type) const
{
    auto filter = [_flags, _type](const std::pair<const std::string, Property*>& pair) -> bool
    {
        auto* property = pair.second;
        return type::is_implicitly_convertible(property->get_type(), _type)
               && ( property->has_flags( _flags ) );
    };

    auto found = std::find_if(m_properties_by_name.begin(), m_properties_by_name.end(), filter );
    if ( found != m_properties_by_name.end())
        return found->second;
    return nullptr;
}

Property* PropertyBag::find_by_name(const char *_name)
{
    return const_cast<Property*>(const_cast<const PropertyBag*>(this)->find_by_name(_name));
}

const Property* PropertyBag::find_by_name(const char *_name) const
{
    auto it = m_properties_by_name.find({_name});
    if( it != m_properties_by_name.end())
    {
        return it->second;
    }
    return nullptr;
}

Property* PropertyBag::at(size_t pos)
{
    return m_properties[pos];
}

const Property* PropertyBag::at(size_t pos) const
{
    return m_properties[pos];
}

Property* PropertyBag::find_id_from_name(const char *_name) const
{
    for(auto& [name, property] : m_properties_by_name )
    {
        if( name == _name)
            return property;
    }
    ASSERT(false)
    return nullptr;
}

Property* PropertyBag::get_this()
{
    return m_properties[THIS_ID];
}

const Property* PropertyBag::get_this() const
{
    return m_properties[THIS_ID];
}

size_t PropertyBag::size() const
{
    return m_properties.size();
}

PropertyBag::~PropertyBag()
{
    for(Property* each_property : m_properties)
        delete each_property;
}
