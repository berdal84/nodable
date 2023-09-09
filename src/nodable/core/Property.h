#pragma once

#include "fw/core/types.h" // for constants and forward declarations
#include "fw/core/reflection/variant.h"
#include "fw/core/Pool.h"
#include "core/Visibility.h"
#include "core/Token.h"
#include "core/Way.h"

#include <string>
#include <vector>
#include <memory> // std::shared_ptr

namespace ndbl
{
    /**
     * @class The class store a value (as a variant) and is owned by a PropertyGroup
     *
     * A property is like a property in OOP, you can set its visibility, type, and value.
     * In Nodable, a property can also be connected (see DirectedEdge) to another property is they "way" allows it.
     */
	class Property
    {
    public:

        typedef int Flags;
        enum Flags_ {
            Flags_none         = 0,
            Flags_initialize   = 1,
            Flags_define       = 1 << 1,
            Flags_reset_value  = 1 << 2
        };

        fw::ID<Property> id;
        Token            token;

        explicit Property();
        explicit Property(const std::string &);
        explicit Property(int);
        explicit Property(bool);
        explicit Property(double);
        explicit Property(const char *);
        template<typename T>
        explicit Property(fw::ID<T> _value);
        ~Property() = default;

        fw::variant*                 operator->() { return value(); }
        const fw::variant*           operator->() const { return value(); }
        fw::variant&                 operator*() { return *value(); }
        const fw::variant&           operator*() const { return *value(); }
        template<typename T> T       to()const { return value()->to<T>(); }

        void digest(Property *_property);
        bool allows_connection(Way _flag)const;
        void set_allowed_connection(Way wayFlags) { m_allowed_connection = wayFlags; }
        void set_name(const char* _name) { m_name = _name; }
        void set(const Property& _other) { value()->set(_other.m_variant); }
        template<typename T>
        void set(T _value);
		void set_type(const fw::type* _type) { value()->ensure_is_type(_type); }
		void set_visibility(Visibility _visibility) { m_visibility = _visibility; }

        const std::string&           get_name()const { return m_name; }
        const fw::type*              get_type()const { return value()->get_type(); }
        Visibility                   get_visibility()const { return m_visibility; }
        Way                          get_allowed_connection()const { return m_allowed_connection; }
        void                         ensure_is_initialized(bool b);
        void                         set_ref() { m_is_ref = true; }
        bool                         is_ref() const;

        template<typename T, typename U = fw::ID<T> >
        bool is_referencing() const
        {
            return get_type()->is<U>();
        }

        fw::variant*                 value()     { return &m_variant; }
        const fw::variant*           value()const{ return &m_variant; }

		static Property*             new_with_type(const fw::type *_type, Flags _flags);
		static std::vector<fw::variant*> get(std::vector<Property *> _in_properties);

        template<typename T>
        T& as() { return value()->as<T>(); }

        template<typename T>
        T as() const { return value()->as<T>(); }

        bool is_type_null() const;

    private:
        Visibility 		        m_visibility;
		Way                     m_allowed_connection;
		std::string             m_name;
		fw::variant             m_variant;
        bool                    m_is_ref;
    };

    template<typename T>
    void Property::set(T _value)
    { value()->set( _value ); }
}