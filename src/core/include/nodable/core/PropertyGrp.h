#pragma once

#include <memory>                 // std::shared_ptr
#include <nodable/core/Property.h>// for Type enum
#include <nodable/core/Visibility.h>
#include <nodable/core/Way.h>
#include <nodable/core/reflection/reflection>
#include <nodable/core/types.h>
#include <set>

namespace ndbl
{

    /**
     * @brief The Properties class is a Property* container for a given Node.
     * This class uses several indexes (by adress, name, insertion order).
     */
	class PropertyGrp
    {
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
        Property*              get_first(Way _way, const type& _type) const;
        Property*              get_input_at(u8_t n) const;
        std::shared_ptr<Property> add(const char *_name, Visibility _visibility, const type& _type, Way, Property::Flags = 0);
        template<typename T>
        std::shared_ptr<Property> add(const char* _name, Visibility _visibility, Way _way, Property::Flags _flags = 0)
            {
                return add(_name, _visibility, type::get<T>(), _way, _flags);
            }
	private:
        Node*                               m_owner;
        std::set<std::shared_ptr<Property>> m_properties;
        PropertiesIndex                     m_properties_;
	};
}