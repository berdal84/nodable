#pragma once

#include "ndbl/core/Token.h"
#include "tools/core/memory/memory.h"
#include "tools/core/reflection/variant.h"
#include "tools/core/types.h"// for constants and forward declarations

#include <string>
#include <vector>

namespace ndbl
{
    // forward declarations
    class Node;

    typedef int PropertyFlags;
    enum PropertyFlag
    {
        PropertyFlag_NONE            = 0,
        PropertyFlag_INITIALIZE      = 1 << 0,
        PropertyFlag_DEFINE          = 1 << 1,
        PropertyFlag_RESET_VALUE     = 1 << 2,
        PropertyFlag_IS_REF          = 1 << 3,
        PropertyFlag_VISIBLE         = 1 << 4,
        PropertyFlag_IS_THIS         = 1 << 5, // Property pointing this Property's parent Node (stored as void* in variant).
        PropertyFlag_VISIBILITY_MASK = PropertyFlag_VISIBLE,
        PropertyFlag_DEFAULT         = PropertyFlag_VISIBLE
    };

    /**
     * @class The class store a value (as a variant) and is owned by a PropertyGroup
     *
     * A property is like a property in OOP, you can set its visibility, type, and value.
     * In Nodable, a property can also be connected (see DirectedEdge) to another property is they "way" allows it.
     */
	class Property
    {
    public:
        Token token;

        explicit Property(Node* = nullptr);
        explicit Property(const std::string &, Node*  = nullptr);
        explicit Property(int, Node* = nullptr);
        explicit Property(bool, Node* = nullptr);
        explicit Property(double, Node* = nullptr);
        explicit Property(const char *, Node* = nullptr);
        explicit Property(const tools::type *_type, PropertyFlags _flags, Node* = nullptr);
        ~Property() = default;

        tools::variant*              operator->() { return value(); }
        const tools::variant*        operator->() const { return value(); }
        tools::variant&              operator*() { return *value(); }
        const tools::variant&        operator*() const { return *value(); }
        template<typename T> T       to()const { return value()->to<T>(); }

        void                         digest(Property *_property);
        bool                         has_flags( PropertyFlags _flags )const;
        void                         set_name(const char* _name) { m_name = _name; }
        void                         set(const Property& _other) { value()->set(_other.m_variant); }
        template<typename T>void     set(T _value);
		void                         set_visibility(PropertyFlags);
        Node*                        get_node()const { return m_node; }
        const std::string&           get_name()const { return m_name; }
        const tools::type*           get_type()const { return value()->get_type(); }
        PropertyFlags                get_visibility()const { return m_flags & PropertyFlag_VISIBILITY_MASK; }
        PropertyFlags                flags()const { return m_flags; }
        void                         flag_as_reference();
        bool                         is_ref() const;
        bool                         is_type(const tools::type *_type) const;
        bool                         is_type_null() const;
        bool                         is_this() const;
        tools::variant*              value()     { return &m_variant; }
        const tools::variant*        value()const{ return &m_variant; }

		static std::vector<tools::variant*> get(std::vector<Property *> _in_properties);

        template<typename T>
        T& as() { return value()->as<T>(); }

        template<typename T>
        T as() const { return value()->as<T>(); }

    private:
        Node*          m_node;
        PropertyFlags  m_flags;
		std::string    m_name;
        tools::variant m_variant;
    };

    template<typename T>
    void Property::set(T _value)
    { value()->set( _value ); }
}