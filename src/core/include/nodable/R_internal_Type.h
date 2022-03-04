#pragma once

#include "R_internal_Meta.h"

namespace Nodable::R
{
    class Type : i_meta
    {
    public:
        Type( const char* _cpp_name, const char* _reflect_name, Typename _reflect_type )
        : m_cpp_name(_cpp_name)
        , m_reflect_name(_reflect_name)
        , m_reflect_type(_reflect_type) {};

        const char* get_name() const override { return m_cpp_name; };
        Typename    get_typename() const override { return m_reflect_type; }
        Qualifier   get_qualifier() const override { return m_qualifier; }
        bool        has_qualifier(Qualifier _other_qualifier) const override;
        void        add_qualifier(Qualifier _other_qualifier) override;
        static Type* s_unknown;
        static bool   is_ptr(const Type*);
        static bool   is_ref(const Type*);
        static Type*  add_ref(Type*);
        static Type*  add_ptr(Type*);
        static bool   is_convertible( const Type* , const Type* );

        bool equals(const Type* _other) const
        {
            return m_qualifier == _other->m_qualifier
                && m_reflect_type == _other->m_reflect_type;
        }

        static Type*  make_ptr(const Type* _type)
        {
            Type* base_copy = new Type(*_type);
            return add_ptr(base_copy);
        }
        static Type*  make_ref(const Type* _type)
        {
            Type* base_copy = new Type(*_type);
            return add_ref(base_copy);
        }

        template<class Dst, class Src>
        static Dst* cast_pointer(Src *_source)
        {
            static_assert( is_class_reflected<Src>::value, "class Src is not reflected by R" );
            static_assert( is_class_reflected<Dst>::value, "class Dst is not reflected by R" );

            if(_source->get_class()->is_child_of(Dst::Get_class()))
                return dynamic_cast<Dst*>(_source);
            return nullptr;
        };

    protected:
        const char* m_cpp_name;
        const char* m_reflect_name;
        Typename m_reflect_type;
        Qualifier m_qualifier;
    };
}