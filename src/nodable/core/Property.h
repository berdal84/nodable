#pragma once

#include "core/Token.h"
#include "fw/core/memory/Pool.h"
#include "fw/core/reflection/variant.h"
#include "fw/core/types.h"// for constants and forward declarations

#include <string>
#include <vector>

namespace ndbl
{
    typedef int PropertyFlags;
    enum PropertyFlag
    {
        PropertyFlag_NONE            = 0,
        PropertyFlag_INITIALIZE      = 1 << 0,
        PropertyFlag_DEFINE          = 1 << 1,
        PropertyFlag_RESET_VALUE     = 1 << 2,
        PropertyFlag_IS_REF          = 1 << 3,
        PropertyFlag_VISIBLE         = 1 << 4,
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
        fw::ID<Property> id;
        Token            token;

        explicit Property();
        explicit Property(const std::string &);
        explicit Property(int);
        explicit Property(bool);
        explicit Property(double);
        explicit Property(const char *);
        explicit Property(const fw::type *_type, PropertyFlags _flags);
        ~Property() = default;

        fw::variant*                 operator->() { return value(); }
        const fw::variant*           operator->() const { return value(); }
        fw::variant&                 operator*() { return *value(); }
        const fw::variant&           operator*() const { return *value(); }
        template<typename T> T       to()const { return value()->to<T>(); }

        void digest(Property *_property);
        bool has_flags( PropertyFlags _flags )const;
        void set_name(const char* _name) { m_name = _name; }
        void set(const Property& _other) { value()->set(_other.m_variant); }
        template<typename T>
        void set(T _value);
		void set_visibility(PropertyFlags);

        const std::string&           get_name()const { return m_name; }
        const fw::type*              get_type()const { return value()->get_type(); }
        PropertyFlags                get_visibility()const { return m_flags & PropertyFlag_VISIBILITY_MASK; }
        PropertyFlags                flags()const { return m_flags; }
        void                         ensure_is_initialized(bool b);
        void                         flag_as_reference();
        bool                         is_ref() const;
        fw::variant*                 value()     { return &m_variant; }
        const fw::variant*           value()const{ return &m_variant; }

		static std::vector<fw::variant*> get(std::vector<Property *> _in_properties);

        template<typename T>
        T& as() { return value()->as<T>(); }

        template<typename T>
        T as() const { return value()->as<T>(); }

        bool is_type_null() const;

        bool is_this() const;

    private:
        PropertyFlags m_flags;
		std::string   m_name;
		fw::variant   m_variant;
    };

    template<typename T>
    void Property::set(T _value)
    { value()->set( _value ); }
}