#pragma once

#include <set>
#include <memory>                 // std::shared_ptr

#include "tools/core/memory/memory.h"
#include "tools/core/reflection/reflection"
#include "tools/core/types.h"

#include "constants.h"
#include "Property.h"
#include "Slot.h"

namespace ndbl
{
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
        PropertyBag() = default;
		PropertyBag(PropertyBag&&) = default;
		~PropertyBag() = default;
        PropertyBag& operator=(PropertyBag&&) = default;

        iterator               begin() { return m_properties.begin(); }
        iterator               end() { return m_properties.end(); }
        const_iterator         begin() const { return m_properties.begin(); }
        const_iterator         end() const { return m_properties.end(); }
        size_t                 size() const;
        bool                   has(const char*) const;
        Property*              at( tools::ID<Property>);
        const Property*        at( tools::ID<Property>) const;
        Property*              find_by_name(const char* _name);
        const Property*        find_by_name(const char* _name) const;
        Property*              find_first( PropertyFlags, const tools::type* );
        const Property*        find_first( PropertyFlags, const tools::type* ) const;
        tools::ID<Property>       find_id_from_name(const char*) const;
        Property*              get_this();
        const Property*        get_this() const;
        tools::ID<Property>       add( const tools::type* _type,
                                    const char *_name,
                                    PropertyFlags = PropertyFlag_DEFAULT );

        template<typename T>
        tools::ID<Property> add(
            const char* _name,
            PropertyFlags _flags = PropertyFlag_DEFAULT  )
        {
            return add( tools::type::get<T>(), _name, _flags);
        }

    private:
        const Property* _find_first( PropertyFlags _flags, const tools::type *_type) const;
        const Property* _find_nth_write_only( u8_t _n ) const;
        std::vector<Property>  m_properties;
        std::map<std::string, tools::ID<Property>> m_properties_by_name;
    };
}