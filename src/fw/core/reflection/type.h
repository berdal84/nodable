#pragma once

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <typeinfo>
#include <memory>
#include <vector>

#include "type_register.h"
#include "core/assertions.h"

namespace fw
{
    // forward declarations
    class iinvokable;
    class iinvokable_nonstatic;

    /** Empty structure to act like any type, @related fw::variant class */
    struct any_t{};
    /** Empty structure to act like a null type, @related fw::variant class */
    struct null_t{};

    /**
     * @struct Removes a pointer from a given type PointerT
     * @example @code
     * using _Class = fw::remove_pointer<Class*>::type; // _Class == Class
     */
    template<typename PointerT>
    struct remove_pointer
    {
        // PointerT without pointer (ex: void* => void, MyClass* => MyClass)
        using type = typename std::remove_pointer< typename std::decay<PointerT>::type>::type;
        constexpr static const char* name() { return typeid(type).name(); };
    };


    /**
     * @struct Check if a type T is a class or not
     * @example @code
     * const my_type_is_a_class = is_class<MyType>::value;
     */
    template<typename T>
    struct is_class
    {
        static constexpr bool value = std::is_class<typename remove_pointer<T>::type>::value
                                      && !std::is_same<any_t, T>::value  // is_class<T>::type::value return true for structs
                                      && !std::is_same<null_t, T>::value;
    };

    /**
     * @class Type descriptor. Holds meta data corresponding to a given type.
     *
     * @example @code
     * // Will get or create the type descriptor for MyType
     * const type* t = type::get<MyType>();
     * // Checks if two type descriptors are the same
     * bool equals = t->is<MyType>(): // is true
     */
    class type
    {
        friend class registration;
        friend class type_register;

    public:
        typedef u8_t Flags;
        enum Flags_ : u8_t
        {
            Flags_NONE       = 0u,
            Flags_IS_CLASS   = 1u,
            Flags_IS_CONST   = 1u << 1,
            Flags_IS_POINTER = 1u << 2,
            Flags_HAS_PARENT = 1u << 3,
            Flags_HAS_CHILD  = 1u << 4,
        };

        type(
            std::size_t _index,
            std::size_t _primitive_index,
            const char* _name,
            const char* _compiler_name,
            Flags _flags);

        type(const type&) = delete; // a type must be unique
        type(type&&) = delete;
        ~type() = default;

        std::size_t               index() const { return m_index; }
        const char*               get_name() const { return m_name; };
        bool                      is_class() const { return m_flags & Flags_IS_CLASS; }
        bool                      any_of(std::vector<const type*> args)const;
        bool                      has_parent() const { return m_flags & Flags_HAS_PARENT; }
        bool                      is_ptr() const { return m_flags & Flags_IS_POINTER; }
        bool                      is_const() const { return m_flags & Flags_IS_CONST; }
        bool                      is_child_of(const type* _possible_parent_class, bool _selfCheck = true) const;
        bool                      equals(const type* other) const { return equals(this, other); }
        void                      add_parent(std::size_t _parent);
        void                      add_child(std::size_t _child);
        void                      add_static(const std::string& _name, std::shared_ptr<iinvokable> _invokable);
        void                      add_method(const std::string& _name, std::shared_ptr<iinvokable_nonstatic> _invokable);
        const std::unordered_set<std::shared_ptr<iinvokable>>&
                                  get_static_methods()const { return m_static_methods; }
        const std::unordered_set<std::shared_ptr<iinvokable_nonstatic>>&
                                  get_methods()const { return m_methods; }
        std::shared_ptr<iinvokable>
                                  get_static(const std::string& _name) const;
        std::shared_ptr<iinvokable_nonstatic>
                                  get_method(const std::string& _name) const;
        template<class T>
        inline bool               is_child_of() const { return is_child_of(get<T>(), true); }
        template<class T>
        inline bool               is_not_child_of() const { return !is_child_of(get<T>(), true); }
        template<typename T>
        bool                      is() const;

        // static

        static bool               is_ptr(const type* type) { return type->is_ptr(); }
        static bool               is_implicitly_convertible(const type* _src, const type* _dst);
        static bool               equals(const type* left, const type* right);
        template<typename T>
        static const type*        get();
        template<typename T>
        static type*              create(const char* _name = "");
        template<typename T>
        static const type*        get(T value) { return get<T>(); }
        static const type*        any();
        static const type*        null();

    protected:
        const char* m_name;
        const char* m_compiler_name;
        Flags       m_flags;
        const std::size_t m_primitive_index; // ex: T
        const std::size_t m_index;           // ex: T**, T*
        std::unordered_set<std::size_t> m_parents;
        std::unordered_set<std::size_t> m_children;
        std::unordered_set<std::shared_ptr<iinvokable>>                        m_static_methods;
        std::unordered_map<std::string, std::shared_ptr<iinvokable>>           m_static_methods_by_name;
        std::unordered_set<std::shared_ptr<iinvokable_nonstatic>>              m_methods;
        std::unordered_map<std::string, std::shared_ptr<iinvokable_nonstatic>> m_methods_by_name;
    };

    template<typename T>
    bool type::is() const
    { return equals(this, get<T>()); }

    template<typename T>
    const type* type::get()
    {
        std::size_t index = typeid(T).hash_code();

        if ( type_register::has(index) )
        {
            return type_register::get(index);
        }

        type* type = create<T>();
        type_register::insert(type);

        return type;
    }

    template<typename T>
    type* type::create(const char* _name)
    {
        Flags flags = Flags_NONE;
        if(std::is_pointer<T>::value) flags += Flags_IS_POINTER;
        if(std::is_const<T>::value)   flags += Flags_IS_CONST;
        if(fw::is_class<T>::value)    flags += Flags_IS_CLASS;

        return new type(
            typeid(T).hash_code(),
            typeid(typename remove_pointer<T>::type).hash_code(), // primitive type
            _name,
            typeid(T).name(), // compiler name
            flags
        );
    }

    /**
     * Return if SourceClass extends PossiblyBaseClass
     */
    template<class PossiblyBaseClass, class SourceClass, bool self_check = true>
    bool extends(SourceClass* source_ptr)
    {
        // ensure both classes are reflected
        static_assert(std::is_member_function_pointer_v<decltype(&SourceClass::get_type)>);
        static_assert(std::is_member_function_pointer_v<decltype(&PossiblyBaseClass::get_type)>);

        // check if source_type is a child of possibly_base_class
        const type* source_type = source_ptr->get_type();
        const type* possibly_base_class = type::get<PossiblyBaseClass>();
        return source_type->is_child_of(possibly_base_class, self_check );
    }

    template<class TargetClass>
    inline TargetClass* cast(TargetClass* source_ptr)
    { return source_ptr; }

    template<class TargetClass, class SourceClass>
    TargetClass* cast(SourceClass* source_ptr)
    {
        static_assert(!std::is_same_v<TargetClass, SourceClass>);

        if( extends<TargetClass>(source_ptr) )
        {
            return static_cast<TargetClass*>(source_ptr);
        }
        return nullptr;
    }

    template<class Type>
    constexpr bool has_get_type = std::is_member_pointer<decltype(&Type::get_type)>::value;

    template<class DerivedT, class BaseT>
    struct is_base_of
    {
        static_assert(fw::has_get_type<BaseT>, "BaseT must have polymorphic reflection");
        static_assert(fw::has_get_type<DerivedT>, "DerivedT must have polymorphic reflection");
        static constexpr bool value = std::is_same_v<DerivedT, BaseT> || std::is_base_of_v<DerivedT, BaseT>;
    };
}