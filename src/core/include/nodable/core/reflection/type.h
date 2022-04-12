#pragma once

#include <unordered_set>
#include <string>

#include "typeregister.h"

namespace Nodable
{
    class type
    {
        friend struct registration;
        friend struct initializer;
    public:
        struct any_t{};
        struct null_t{};
        using hash_code_t = size_t;

    private:
        type() = default;
    public:
        ~type() = default;

        const char*               get_name() const { return m_name.c_str(); };
        std::string               get_fullname() const;
        size_t                    hash_code() const { return m_hash_code; }
        type                      get_underlying_type() const;
        template<class T> bool    is_exactly() const { return *this == type::get<T>(); }
        template<class T> bool    is_not() const { return  *this != type::get<T>(); }
        bool                      is_class() const { return m_is_class; }
        bool                      is_ptr() const;
        bool                      is_ref() const;
        bool                      is_const() const;
        bool                      is_child_of(type _possible_parent_class, bool _selfCheck = true) const;
        void                      add_parent(hash_code_t _parent);
        void                      add_child(hash_code_t _child);
        template<class T> inline bool is_child_of() const { return is_child_of(get<T>(), true); }
        template<class T> inline bool is_not_child_of() const { return !is_child_of(get<T>(), true); }

        static bool               is_ptr(type);
        static bool               is_ref(type);
        static bool               is_implicitly_convertible(type _left, type _right);

        friend bool operator==(const type& left, const type& right)
        {
            return left.m_hash_code == right.m_hash_code;
        }

        friend bool operator!=(const type& left, const type& right)
        {
            return left.m_hash_code != right.m_hash_code;
        }

        /** to get a type at compile time */
        template<typename T>
        static type get()
        {
            using unqualified_T = typename std::decay<T>::type;
            using noptr_T       = typename std::remove_pointer<unqualified_T>::type;
            type t = typeregister::get(typeid(noptr_T).hash_code());
            t.m_is_pointer   = std::is_pointer<T>();
            t.m_is_reference = std::is_reference<T>();
            t.m_is_const     = std::is_const<T>();
            return t;
        }

        /** to get a type ar runtime */
        template<typename T>
        static type get(T value) { return get<T>(); }

        static type any;
        static type null;
    protected:
        std::string m_name;
        bool        m_is_class;
        bool        m_is_pointer;
        bool        m_is_reference;
        bool        m_is_const;
        hash_code_t m_hash_code;
        hash_code_t m_underlying_type;
        std::unordered_set<hash_code_t> m_parents;
        std::unordered_set<hash_code_t> m_children;
    };
}