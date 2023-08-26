#include "PropertyBag.h"
#include "Node.h"

using namespace ndbl;

PropertyBag::PropertyBag()
{
}

PropertyBag::~PropertyBag()
{
    for ( auto* each_property : m_properties )
    {
        delete each_property;
    }
}

bool PropertyBag::has(const char* _name) const
{
    return m_properties_.by_name.find(_name) != m_properties_.by_name.end();
}

Property* PropertyBag::add(const fw::type* _type, const char* _name, Visibility _visibility, Way _way, Property::Flags _flags )
{
    FW_ASSERT(!has(_name));

    // create the property
    Property* new_property = Property::new_with_type(_type, _flags);
    new_property->m_owner = m_owner;
    new_property->set_name(_name);
    new_property->set_visibility(_visibility);
    new_property->set_allowed_connection(_way);

    // register property
    m_properties.insert(new_property);

    // add to indexes
    m_properties_.by_name.insert({new_property->get_name(), new_property});
    m_properties_.by_index.push_back(new_property);

    return new_property;
}

Property* PropertyBag::get_first(Way _way, const fw::type *_type) const
{
    auto filter = [_way, _type](auto& each) -> bool
    {
        auto [_, property] = each;
        return fw::type::is_implicitly_convertible(property->get_type(), _type)
               && ( property->get_allowed_connection() & _way );
    };

    auto found = std::find_if(m_properties_.by_name.begin(), m_properties_.by_name.end(), filter );
    if ( found != m_properties_.by_name.end())
        return found->second;
    return nullptr;
}

Property* PropertyBag::get_input_at(size_t _position) const
{
    size_t count = 0;
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

void PropertyBag::set_owner(ID<Node> owner)
{
    m_owner = owner;

    // Add a property acting like a "this" for the owner Node.
    auto this_property = add<ID<Node>>(k_this_property_name, Visibility::Always, Way::Way_Out);
    this_property->set( owner );

    for(auto each_property : m_properties )
    {
        each_property->m_owner = owner;
    }
}

PropertyBag::PropertyBag(PropertyBag&& other)
{
    *this = std::move(other);
}

PropertyBag& PropertyBag::operator=(PropertyBag&& other )
{
    m_properties_ = other.m_properties_;
    m_properties  = std::move(other.m_properties);
    m_owner       = other.m_owner;

    return *this;
}
