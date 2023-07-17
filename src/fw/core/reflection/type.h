#pragma once

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <typeinfo>
#include <typeindex>
#include <memory>
#include <vector>

#include "type_register.h"
#include "core/assertions.h"

namespace fw
{
    struct any_t{};
    struct null_t{};
    using hash_code_t = std::type_index;
    class iinvokable;
    class iinvokable_nonstatic;

    template<typename T>
    struct unqualified
    {
        using  type = typename std::remove_pointer< typename std::decay<T>::type>::type;
        constexpr static const char* name() { return typeid(type).name(); };
        constexpr static size_t hash_code() { return typeid(type).hash_code(); };
    };

    template<typename T>
    struct is_class
    {
        static constexpr bool value =       std::is_class<typename unqualified<T>::type>::value
                                            && !std::is_same<any_t, T>::value
                                            && !std::is_same<null_t, T>::value;
    };

    class type
    {
        friend class registration;
        friend class type_register;

    public:
        type(std::type_index index, std::type_index primitive_index)
        : m_index(index)
        , m_primitive_index(primitive_index)
        { }

        type(const type&) = delete; // a type must be unique
        type(type&&) = delete;
        ~type() = default;

        const char*               get_name() const { return m_name.c_str(); };
        std::string               get_fullname() const;
        std::type_index           index() const { return m_index; }
        bool                      is_class() const { return m_is_class; }
        bool                      any_of(std::vector<const type*> args)const;
        bool                      is_ptr() const;
        bool                      is_const() const;
        bool                      is_child_of(const type* _possible_parent_class, bool _selfCheck = true) const;
        bool                      equals(const type* other) const { return equals(this, other); }
        void                      add_parent(hash_code_t _parent);
        void                      add_child(hash_code_t _child);
        void                      add_static(const std::string& _name, std::shared_ptr<iinvokable> _invokable);
        void                      add_method(const std::string& _name, std::shared_ptr<iinvokable_nonstatic> _invokable);
        const std::unordered_set<std::shared_ptr<iinvokable>>&
                                  get_static_methods()const { return m_static_methods; }
        const std::unordered_set<std::shared_ptr<iinvokable_nonstatic>>&
                                  get_methods()const { return m_methods; }
        std::shared_ptr<iinvokable> get_static(const std::string& _name) const;
        std::shared_ptr<iinvokable_nonstatic> get_method(const std::string& _name) const;
        template<class T> inline bool is_child_of() const { return is_child_of(get<T>(), true); }
        template<class T> inline bool is_not_child_of() const { return !is_child_of(get<T>(), true); }
        template<typename T>
        bool is() const
        { return equals(this, get<T>()); }

        static bool               is_ptr(const type*);
        static bool               is_implicitly_convertible(const type* _src, const type* _dst);

        static bool equals(const type* left, const type* right)
        {
            return left->m_index == right->m_index
                 && left->m_is_pointer   == right->m_is_pointer
                 && left->m_is_const     == right->m_is_const;
        }

        template<typename T>
        static const type* get()
        {
            auto type_index = std::type_index(typeid(T));

            if( type_register::has(type_index) )
            {
                return type_register::get(type_index);
            }

            type* type = create<T>();
            type_register::insert(type);

            return type;
        }

        template<typename T>
        static type* create(const std::string &_name = "")
        {
            using primitive_T = typename unqualified<T>::type;
            auto* new_type = new type(std::type_index(typeid(T)),
                                      std::type_index(typeid(primitive_T)));
            
            new_type->m_name          = _name;
            new_type->m_compiler_name = typeid(T).name();
            new_type->m_is_pointer    = std::is_pointer<T>::value;
            new_type->m_is_const      = std::is_const<T>::value;
            new_type->m_is_class      = fw::is_class<T>::value;

            return new_type;
        }

        template<typename T>
        static const type* get(T value) { return get<T>(); }
        static const type* any();
        static const type* null();
    protected:
        std::string m_name;
        std::string m_compiler_name;
        bool        m_is_class;
        bool        m_is_pointer;
        bool        m_is_const;
        const std::type_index m_primitive_index; // ex: T
        const std::type_index m_index;           // ex: T**, T&, T&&
        std::unordered_set<hash_code_t> m_parents;
        std::unordered_set<hash_code_t> m_children;
        std::unordered_set<std::shared_ptr<iinvokable>>                    m_static_methods;
        std::unordered_map<std::string, std::shared_ptr<iinvokable>>       m_static_methods_by_name;
        std::unordered_set<std::shared_ptr<iinvokable_nonstatic>>              m_methods;
        std::unordered_map<std::string, std::shared_ptr<iinvokable_nonstatic>> m_methods_by_name;
    };

    template<class target_t, class source_t>
    static target_t* cast(source_t *_source)
    {
        if( _source->get_type()->is_child_of( type::get<target_t>(), true ))
        {
            return static_cast<target_t*>(_source);
        }
        return nullptr;
    }
}