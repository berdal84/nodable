#pragma once

#include <set>
#include <memory>                 // std::shared_ptr

#include "fw/core/reflection/reflection"
#include "fw/core/types.h"

#include "Property.h"
#include "Visibility.h"
#include "Way.h"

namespace ndbl
{

    /**
     * @brief The Properties class is a Property* container for a given Node.
     * This class uses several indexes (by adress, name, insertion order).
     */
	class PropertyGrp
    {
        using PropertyVec = std::vector<Property*>;
        using PropertyMap = std::map<std::string, Property*>;
    private:
        struct PropertiesIndex {
            PropertyVec  by_index;   // properties ordered
            PropertyMap  by_name;    // properties indexed by name
        };
	public:
		explicit PropertyGrp(Node* owner);
		virtual ~PropertyGrp();

        bool                   has(const char*) const;
        Property*              get(const char* _name)const { return m_properties_.by_name.at(_name); };
        Node*                  get_owner()const { return m_owner; };
		const PropertyMap&     by_name()const { return m_properties_.by_name; };
		const PropertyVec&     by_index()const { return m_properties_.by_index; };
        Property*              get_first(Way _way, const fw::type *_type) const;
        Property*              get_input_at(u64_t _position) const;
        std::shared_ptr<Property> add(                                      // add a new property with a given name and type.
                const fw::type* _type,
                const char *_name,
                Visibility _visibility = Visibility::Default,
                Way _way = Way_Default,
                Property::Flags = 0);
        template<typename T>
        std::shared_ptr<Property> add(                                      // add a new property with a given name and a static type.
                const char* _name,
                Visibility _visibility = Visibility::Default,
                Way _way = Way_Default,
                Property::Flags _flags = 0)
            {
                return add(fw::type::get<T>(), _name, _visibility, _way, _flags);
            }
	private:
        Node*                               m_owner;
        std::set<std::shared_ptr<Property>> m_properties;
        PropertiesIndex                     m_properties_;
	};
}