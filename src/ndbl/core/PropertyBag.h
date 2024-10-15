#pragma once

#include <set>
#include <memory>                 // std::shared_ptr

#include "tools/core/reflection/Type.h"
#include "tools/core/types.h"

#include "constants.h"
#include "Property.h"

namespace ndbl
{
    // forward declarations
    class Node;

    /**
     * @brief The Properties class is a Property* container for a given Node.
     * This class uses several indexes (by address, name, insertion order).
     */
	class PropertyBag
    {
    public:
        static constexpr size_t THIS_ID = 0; // id of the "this" Property. A "this" Property points to its owner's ID<Node>
        typedef std::vector<Property*>::iterator       iterator;
        typedef std::vector<Property*>::const_iterator const_iterator;

        ~PropertyBag();
        void             init(Node* owner) { m_owner = owner; }
        iterator         begin() { return m_properties.begin(); }
        iterator         end() { return m_properties.end(); }
        const_iterator   begin() const { return m_properties.begin(); }
        const_iterator   end() const { return m_properties.end(); }
        size_t           size() const;
        bool             has(const char*) const;
        Property*        at( size_t pos );
        const Property*  at( size_t pos ) const;
        Property*        find_by_name(const char* _name);
        const Property*  find_by_name(const char* _name) const;
        Property*        find_first( PropertyFlags, const tools::TypeDescriptor* );
        const Property*  find_first( PropertyFlags, const tools::TypeDescriptor* ) const;
        Property*        find_id_from_name(const char*) const;
        Property*        get_this();
        const Property*  get_this() const;
        Property*        add(const tools::TypeDescriptor* _type, const char *_name, PropertyFlags = PropertyFlag_NONE );
        Property*        add(Property* property);

        template<typename T>
        Property* add( const char* _name, PropertyFlags _flags = PropertyFlag_NONE )
        { return add(tools::type::get<T>(), _name, _flags); }

    private:
        const Property* _find_first( PropertyFlags _flags, const tools::TypeDescriptor *_type) const;
        Node*                   m_owner;
        std::vector<Property*>  m_properties;
        std::map<std::string, Property*> m_properties_by_name;
    };
}