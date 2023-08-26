#pragma once

#include <set>
#include <memory>                 // std::shared_ptr

#include "fw/core/reflection/reflection"
#include "fw/core/types.h"
#include "fw/core/Pool.h"

#include "Property.h"
#include "Visibility.h"
#include "Way.h"

namespace ndbl
{
    using fw::pool::ID;

    /**
     * @brief The Properties class is a Property* container for a given Node.
     * This class uses several indexes (by adress, name, insertion order).
     */
	class PropertyBag
    {
    public:
        using PropertyVec = std::vector<Property*>;
        using PropertyMap = std::map<std::string, Property*>;

		PropertyBag();
		PropertyBag(PropertyBag&&);
		PropertyBag& operator=(PropertyBag&&);
		virtual ~PropertyBag();

        bool                   has(const char*) const;
        Property*              get(const char* _name)const { return m_properties_.by_name.at(_name); };
		const PropertyMap&     by_name()const { return m_properties_.by_name; };
		const PropertyVec&     by_index()const { return m_properties_.by_index; };
        Property*              get_first(Way _way, const fw::type *_type) const;
        Property*              get_input_at(size_t _position) const;
        Property*              add( const fw::type* _type,
                                    const char *_name,
                                    Visibility _visibility = Visibility::Default,
                                    Way _way = Way_Default,
                                    Property::Flags _flags = 0);

        template<typename T>
        Property* add(
            const char* _name,
            Visibility _visibility = Visibility::Default,
            Way _way = Way_Default,
            Property::Flags _flags = 0)
        {
            return add(fw::type::get<T>(), _name, _visibility, _way, _flags);
        }

        void set_owner(ID<Node> owner);
	private:
        struct PropertiesIndex {
            PropertyVec  by_index;   // properties ordered
            PropertyMap  by_name;    // properties indexed by name
        };
        ID<Node>            m_owner;
        std::set<Property*> m_properties;
        PropertiesIndex     m_properties_;
	};
}