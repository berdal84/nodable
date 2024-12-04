#include "ASTNodePropertyBag.h"
#include "ASTNode.h"

using namespace ndbl;
using namespace tools;

bool ASTNodePropertyBag::has(const char* _name) const
{
    return m_properties_by_name.find(_name) != m_properties_by_name.end();
}

ASTNodeProperty* ASTNodePropertyBag::add(ASTNodeProperty* property)
{
    m_properties.push_back(property);
    // Index by name
    m_properties_by_name.insert({property->name(), property});

    return property;
}

ASTNodeProperty* ASTNodePropertyBag::add(const TypeDescriptor* _type, const char* _name, PropertyFlags _flags )
{
    // guards
    VERIFY(m_owner != nullptr , "PropertyBag must be initialized");
    VERIFY(!has(_name)        , "Property name already used");

    // create the property

    auto* new_property = new ASTNodeProperty();
    new_property->init(_type, _flags, m_owner, _name);

    return add(new_property);
}

const ASTNodeProperty* ASTNodePropertyBag::find_first(PropertyFlags _flags, const TypeDescriptor *_type) const
{
    return _find_first( _flags, _type );
}

ASTNodeProperty* ASTNodePropertyBag::find_first(PropertyFlags _flags, const TypeDescriptor *_type)
{
    return const_cast<ASTNodeProperty*>( _find_first(_flags, _type ) );
}

const ASTNodeProperty* ASTNodePropertyBag::_find_first(PropertyFlags _flags, const TypeDescriptor *_type) const
{
    auto filter = [_flags, _type](const std::pair<const std::string, ASTNodeProperty*>& pair) -> bool
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

ASTNodeProperty* ASTNodePropertyBag::find_by_name(const char *_name)
{
    return const_cast<ASTNodeProperty*>(const_cast<const ASTNodePropertyBag*>(this)->find_by_name(_name));
}

const ASTNodeProperty* ASTNodePropertyBag::find_by_name(const char *_name) const
{
    auto it = m_properties_by_name.find({_name});
    if( it != m_properties_by_name.end())
    {
        return it->second;
    }
    return nullptr;
}

ASTNodeProperty* ASTNodePropertyBag::at(size_t pos)
{
    return m_properties[pos];
}

const ASTNodeProperty* ASTNodePropertyBag::at(size_t pos) const
{
    return m_properties[pos];
}

ASTNodeProperty* ASTNodePropertyBag::find_id_from_name(const char *_name) const
{
    for(auto& [name, property] : m_properties_by_name )
    {
        if( name == _name)
            return property;
    }
    ASSERT(false);
    return nullptr;
}

ASTNodeProperty* ASTNodePropertyBag::get_this()
{
    return m_properties[THIS_ID];
}

const ASTNodeProperty* ASTNodePropertyBag::get_this() const
{
    return m_properties[THIS_ID];
}

size_t ASTNodePropertyBag::size() const
{
    return m_properties.size();
}

ASTNodePropertyBag::~ASTNodePropertyBag()
{
    for(ASTNodeProperty* each_property : m_properties)
        delete each_property;
}
