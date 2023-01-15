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
	public:
		explicit PropertyGrp(Node* owner);
		virtual ~PropertyGrp();
        template<typename T>
        Property * add(const char* _name, Visibility _visibility, Way _way, Property::Flags _flags = 0)
        {
            return add(_name, _visibility, type::get<T>(), _way, _flags);
        }
        Property *             add(const char *_name, Visibility _visibility, type _type, Way, Property::Flags = 0);
		// void                remove(const char*); // if we implement that, we have to think about all indexes.
		// void                remove(Property*);     // => implement remove_from_indexes(Property*)
        bool                has(const char*);
		bool                has(const Property *);
        Property *             get(const char* _name)const { return m_properties_by_name.at(_name); };
        Node*               get_owner()const { return m_owner; };
		const PropertyMap &    by_name()const { return m_properties_by_name; };
		const PropertyVec &    by_id()const { return m_properties_by_id; };
        Property *             get_first(Way _way, type _type) const;
        Property *             get_input_at(u8_t n) const;
	private:
	    void                add_to_indexes(Property *);
	    // void                remove_from_indexes(Property*);

	    Node*             m_owner;
        PropertyMap m_properties_by_name; // tripple index, by name ...
        std::set<Property *> m_properties;         // ... by adress ...
        PropertyVec m_properties_by_id;   // ... and by id (insertion order).
	};
}