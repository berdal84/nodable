#pragma once

#include <set>
#include <memory>                 // std::shared_ptr

#include "fw/core/reflection/reflection"
#include "fw/core/types.h"
#include "fw/core/Pool.h"

#include "constants.h"
#include "Property.h"
#include "Visibility.h"
#include "Way.h"

namespace ndbl
{
    using fw::pool::PoolID;

    /**
     * @brief The Properties class is a Property* container for a given Node.
     * This class uses several indexes (by address, name, insertion order).
     */
	class PropertyBag
    {
    public:
        static constexpr size_t THIS_ID = 0; // id of the "this" Property. A "this" Property points to its owner's ID<Node>
        using iterator = std::vector<Property>::iterator;
        using const_iterator = std::vector<Property>::const_iterator;

        template<typename OwnerT>
        PropertyBag(PoolID<OwnerT>)
        {
            static_assert__is_pool_registrable<OwnerT>();
            // Add a property acting like a "this" for the owner Node.
            size_t id = add<PoolID<OwnerT>>(THIS_PROPERTY, Visibility::Always, Way::Out);
            FW_EXPECT(id == THIS_ID, "P_THIS should have a null id");
        }
		PropertyBag(PropertyBag&&);
		PropertyBag& operator=(PropertyBag&&);
		virtual ~PropertyBag();

        iterator               begin() { return m_properties.begin(); }
        iterator               end() { return m_properties.end(); }
        const_iterator         begin() const { return m_properties.begin(); }
        const_iterator         end() const { return m_properties.end(); }
        bool                   has(const char*) const;
        Property*              at(fw::ID<Property>);
        const Property*        at(fw::ID<Property>) const;
        Property*              get(const char* _name);
        const Property*        get(const char* _name) const;
        Property*              get_first(Way _way, const fw::type *_type);
        const Property*        get_first(Way _way, const fw::type *_type) const;
        Property*              get_input_at(fw::ID<Property>);
        const Property*        get_input_at(fw::ID<Property>) const;
        fw::ID<Property>       get_id(const char* _name) const;
        Property*              get_this();
        const Property*        get_this() const;
        fw::ID<Property> add( const fw::type* _type,
                                    const char *_name,
                                    Visibility _visibility = Visibility::Default,
                                    Way _way = Way::Default,
                                    Property::Flags _flags = 0);

        template<typename T>
        fw::ID<Property> add(
            const char* _name,
            Visibility _visibility = Visibility::Default,
            Way _way = Way::Default,
            Property::Flags _flags = 0)
        {
            return add(fw::type::get<T>(), _name, _visibility, _way, _flags);
        }
	private:
        std::vector<Property>  m_properties;
        std::map<std::string, fw::ID<Property>> m_properties_by_name;
    };
}