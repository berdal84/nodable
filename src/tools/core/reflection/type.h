#pragma once

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <typeinfo>
#include <vector>

#include "type_register.h"
#include "tools/core/assertions.h"

namespace tools
{
    // forward declarations
    class IInvokable;
    class IInvokableMethod;

    /** Empty structure to act like any type, @related tools::variant class */
    struct any_t{};
    /** Empty structure to act like a null type, @related tools::variant class */
    struct null_t{};

    template<class T, typename GET_CLASS = decltype(&T::get_class)>
    constexpr bool IsReflectedClass = std::is_member_function_pointer_v<GET_CLASS>;

    /**
     * @struct Removes a pointer from a given type PointerT
     * @example @code
     * using _Class = tools::remove_pointer<Class*>::type; // _Class == Class
     */
    template<typename PointerT>
    struct remove_pointer
    {
        // PointerT without pointer (ex: void* => void, MyClass* => MyClass)
        using type = typename std::remove_pointer< typename std::decay<PointerT>::type>::type;
        constexpr static const char* name() { return typeid(type).name(); };
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
        friend class type_register;
    public:
        using id_t = std::type_index;
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
                id_t _id,
                id_t _primitive_id,
                const char* _name,
                const char* _compiler_name,
                Flags _flags);

        type(const type&) = delete; // a type must be unique
        type(type&&) = delete;
        ~type();

        id_t                      id() const { return m_id; }
        const char*               get_name() const { return m_name; };
        bool                      is_class() const { return m_flags & Flags_IS_CLASS; }
        bool                      any_of(std::vector<const type*> args)const;
        bool                      has_parent() const { return m_flags & Flags_HAS_PARENT; }
        bool                      is_ptr() const { return m_flags & Flags_IS_POINTER; }
        bool                      is_const() const { return m_flags & Flags_IS_CONST; }
        bool                      is_child_of(std::type_index _possible_parent_id, bool _selfCheck = true) const;
        bool                      equals(const type* other) const { return equals(this, other); }
        void                      add_parent(id_t _parent);
        void                      add_child(id_t _child);
        void                      add_static(const std::string& _name, const IInvokable* _invokable);
        void                      add_method(const std::string& _name, const IInvokableMethod* _invokable);
        const std::unordered_set<const IInvokable*>&
                                  get_static_methods()const { return m_static_methods; }
        const std::unordered_set<const IInvokableMethod*>&
                                  get_methods()const { return m_methods; }
        const IInvokable*         get_static(const std::string& _name) const;
        const IInvokableMethod*   get_method(const std::string& _name) const;
        template<class T>
        inline bool               is_child_of() const { return is_child_of(std::type_index(typeid(T)), true); }
        template<class T>
        inline bool               is_not_child_of() const { return !is_child_of<T>(); }
        template<typename T>
        bool                      is() const;

        // static

        static bool               is_ptr(const type* type) { return type->is_ptr(); }
        static bool               is_implicitly_convertible(const type* _src, const type* _dst);
        static bool               equals(const type* left, const type* right);
        template<typename T>
        static const type*        get();

        template<class ...Types>
        struct get_all
        {
            using tuple = std::tuple<Types...>;
            static constexpr size_t size = std::tuple_size_v<tuple>;
            using result_t = std::array<const tools::type*, size>;

            static result_t types()
            {
                result_t output{};
                return extract_type_at<size - 1>(output);
            }

        private:
            template<size_t N, typename std::enable_if_t<N!=0, bool> = 0 >
            static result_t& extract_type_at(result_t& output)
            {
                using element_t = std::tuple_element_t<N, tuple>;
                output[N] = tools::type::get<element_t>();
                return extract_type_at<N - 1>(output);
            }

            template<size_t N, typename std::enable_if_t<N==0, bool> = 0 >
            static result_t& extract_type_at(result_t& output)
            {
                using element_t = std::tuple_element_t<0, tuple>;
                output[0] = tools::type::get<element_t>();
                return output;
            }
        };

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
        const id_t m_primitive_id; // ex: T
        const id_t m_id;           // ex: T**, T*
        std::unordered_set<id_t> m_parents;
        std::unordered_set<id_t> m_children;
        std::unordered_set<const IInvokable*>                    m_static_methods;
        std::unordered_map<std::string, const IInvokable*>       m_static_methods_by_name;
        std::unordered_set<const IInvokableMethod*>              m_methods;
        std::unordered_map<std::string, const IInvokableMethod*> m_methods_by_name;
    };

    template<typename T>
    type::id_t get_type_id()
    { return std::type_index(typeid(T)); }

    template<typename T>
    type::id_t get_primitive_type_id()
    { return get_type_id<typename remove_pointer<T>::type>(); }

    template<typename T>
    const char* get_type_compiler_name()
    { return typeid(T).name(); }

    template<typename T>
    bool type::is() const
    { return equals(this, get<T>()); }

    template<typename T>
    const type* type::get()
    {
        type::id_t id = get_type_id<T>();

        if ( type_register::has(id) )
        {
            return type_register::get(id);
        }

        type* type = create<T>();
        type_register::insert(type);

        return type;
    }

    template<typename T>
    type* type::create(const char* _name)
    {
        Flags flags = Flags_NONE;
        if(std::is_pointer_v<T>) flags |= Flags_IS_POINTER;
        if(std::is_const_v<T>)   flags |= Flags_IS_CONST;
        if(std::is_class_v<T>)   flags |= Flags_IS_CLASS;

        return new type(
            get_type_id<T>(),
            get_primitive_type_id<T>(),
            _name,
            get_type_compiler_name<T>(),
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
        static_assert(IsReflectedClass<SourceClass>);
        static_assert(IsReflectedClass<PossiblyBaseClass>);

        // check if source_type is a child of possibly_base_class
        const type* source_type = source_ptr->get_class();
        const type* possibly_base_class = type::get<PossiblyBaseClass>();
        return source_type->is_child_of(possibly_base_class->id(), self_check );
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

    template<class T, class>
    T* cast(T* source_ptr)
    {
        return source_ptr;
    }

    template<class Type>
    constexpr bool has_get_type = std::is_member_pointer<decltype(&Type::get_type)>::value;

    template<class DerivedT, class BaseT>
    struct is_base_of
    {
        static_assert( tools::has_get_type<BaseT>, "BaseT must have polymorphic reflection");
        static_assert( tools::has_get_type<DerivedT>, "DerivedT must have polymorphic reflection");
        static constexpr bool value = std::is_same_v<DerivedT, BaseT> || std::is_base_of_v<DerivedT, BaseT>;
    };
}