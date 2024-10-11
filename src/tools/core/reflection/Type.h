#pragma once

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <typeinfo>
#include <vector>

#include "TypeRegister.h"
#include "tools/core/assertions.h"

// add this macro to a class declaration to enable reflection on it
#define REFLECT_BASE_CLASS() \
public:\
    virtual const tools::ClassDesc* get_class() const \
    { return tools::type::get_class(this); }\
private:

// add this macro to a class declaration to enable reflection on it.
// Must have a parent class having REFLECT_BASE_CLASS macro.
#define REFLECT_DERIVED_CLASS() \
public:\
    virtual const tools::ClassDesc* get_class() const override \
    { return tools::type::get_class(this); }\
private:

namespace tools
{
    // forward declarations
    class FuncType;
    class IInvokable;
    class IInvokableMethod;
    class TypeDesc;
    class ClassDesc;

    /** Empty structure to act like any type, @related tools::variant class */
    struct any_t{};
    /** Empty structure to act like a null type, @related tools::variant class */
    struct null_t{};

    // Return true if T is reflected
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

    typedef u8_t TypeFlags;
    enum TypeFlag_ : u8_t
    {
        TypeFlag_NONE       = 0u,
        TypeFlag_IS_CLASS   = 1u,
        TypeFlag_IS_CONST   = 1u << 1,
        TypeFlag_IS_POINTER = 1u << 2,
        TypeFlag_HAS_PARENT = 1u << 3,
        TypeFlag_HAS_CHILD  = 1u << 4,
        TypeFlag_IS_MEMBER_PTR = 1u << 5,
    };

    // Type utilities
    namespace type
    {
        bool               is_implicitly_convertible(const TypeDesc* _src, const TypeDesc* _dst);
        bool               equals(const TypeDesc* left, const TypeDesc* right);
        const TypeDesc*    any();
        const TypeDesc*    null();

        template<typename T> std::type_index    id();
        template<typename T> std::type_index    primitive_id();
        template<typename T> const char*        compiler_name();
        template<typename T> TypeFlags          flags();
        template<typename T> const TypeDesc*    get();
        template<typename T> const ClassDesc*   get_class(T* ptr);
        template<typename T> const ClassDesc*   get_class();
        template<typename T> TypeDesc*          create(const char* _name = "");
        template<typename T> const TypeDesc*    get(T value) { return get<T>(); }
        
//        template<class ...Types>
//        struct get_all
//        {
//            using tuple = std::tuple<Types...>;
//            static constexpr size_t size = std::tuple_size_v<tuple>;
//            using result_t = std::array<const tools::TypeDesc*, size>;
//
//            static result_t types()
//            {
//                result_t output{};
//                return extract_type_at<size - 1>(output);
//            }
//
//        private:
//            template<size_t N, typename std::enable_if_t<N!=0, bool> = 0 >
//            static result_t& extract_type_at(result_t& output)
//            {
//                using element_t = std::tuple_element_t<N, tuple>;
//                output[N] = tools::type::get<element_t>();
//                return extract_type_at<N - 1>(output);
//            }
//
//            template<size_t N, typename std::enable_if_t<N==0, bool> = 0 >
//            static result_t& extract_type_at(result_t& output)
//            {
//                using element_t = std::tuple_element_t<0, tuple>;
//                output[0] = tools::type::get<element_t>();
//                return output;
//            }
//        };
    };

    template<typename T>
    std::type_index type::id()
    { return std::type_index(typeid(T)); }

    template<typename T>
    std::type_index type::primitive_id()
    { return id<typename remove_pointer<T>::type>(); }

    template<typename T>
    const char* type::compiler_name()
    { return typeid(T).name(); }

    /**
     * @class TypeDesc (type descriptor) holds meta data relative to a given type.
     *
     * @example @code
     * const TypeDesc* t = type::get<int>();
     * assert( t->is_ptr() == false );
     */
    class TypeDesc
    {
        friend class  TypeRegister;
    public:
        
        TypeDesc(
            std::type_index _id,
            std::type_index _primitive_id,
            const char*     _name,
            const char*     _compiler_name,
            TypeFlags       _flags);

        TypeDesc(const TypeDesc&) = delete; // a type must be unique
        TypeDesc(TypeDesc&&) = delete;
        virtual ~TypeDesc() {};

        std::type_index           id() const { return m_id; }
        const char*               get_name() const { return m_name; };
        bool                      is_class() const { return m_flags & TypeFlag_IS_CLASS; }
        bool                      any_of(std::vector<const TypeDesc*> args)const;
        bool                      has_parent() const { return m_flags & TypeFlag_HAS_PARENT; }
        bool                      is_ptr() const { return m_flags & TypeFlag_IS_POINTER; }
        bool                      is_const() const { return m_flags & TypeFlag_IS_CONST; }
        bool                      equals(const TypeDesc* other) const { return type::equals(this, other); }
        template<typename T>
        bool                      is() const;
        bool                      is_implicitly_convertible(const TypeDesc* _dst ) const;
    protected:
        const char* m_name;
        const char* m_compiler_name;
        TypeFlags   m_flags;
        const std::type_index m_primitive_id; // ex: int
        const std::type_index m_id;           // ex: int**, int*
    };

    /**
     * @class ClassDesc (class descriptor) holds meta data relative to a given class.
     *
     * @example @code
     * const TypeDesc* class_desc = type::get<std::string>();
     * assert( class_desc->is_class() );
     */
    class ClassDesc : public TypeDesc
    {
        friend class TypeRegister;
    public:

        ClassDesc(
            std::type_index _id,
            std::type_index _primitive_id,
            const char*     _name,
            const char*     _compiler_name,
            TypeFlags       _flags);

        ClassDesc(const ClassDesc&) = delete; // a type must be unique
        ClassDesc(ClassDesc&&) = delete;
        ~ClassDesc();

        void                      add_parent(std::type_index _parent);
        void                      add_child(std::type_index _child);
        void                      add_static(const char* _name, const IInvokable*);
        void                      add_method(const char* _name, const IInvokableMethod*);
        const std::unordered_set<const IInvokable*>&       get_statics()const { return m_static_methods; }
        const std::unordered_set<const IInvokableMethod*>& get_methods()const { return m_methods; }
        const IInvokable*         get_static(const char* _name) const;
        const IInvokableMethod*   get_method(const char* _name) const;
        bool                      is_child_of(std::type_index _possible_parent_id, bool _selfCheck = true) const;
        template<class T>
        inline bool               is_child_of() const { return is_child_of(std::type_index(typeid(T)), true); }
        template<class T>
        inline bool               is_not_child_of() const { return !is_child_of<T>(); }

    protected:
        std::unordered_set<std::type_index>                      m_parents;
        std::unordered_set<std::type_index>                      m_children;
        std::unordered_set<const IInvokable*>                    m_static_methods;
        std::unordered_map<std::string, const IInvokable*>       m_static_methods_by_name;
        std::unordered_set<const IInvokableMethod*>              m_methods;
        std::unordered_map<std::string, const IInvokableMethod*> m_methods_by_name;
    };

    template<typename T>
    bool TypeDesc::is() const
    { return type::equals(this, type::get<T>()); }

    template<typename T>
    const ClassDesc* type::get_class()
    {
        static_assert( std::is_class_v<T> );
        return (const ClassDesc*)get<T>();
    }

    template<typename T>
    const ClassDesc* type::get_class(T* ptr)
    {
        static_assert( std::is_class_v<T> );
        return (const ClassDesc*)get<T>();
    }

    template<typename T>
    const TypeDesc* type::get()
    {
        auto id = type::id<T>();

        if ( TypeRegister::has(id) )
        {
            return TypeRegister::get(id);
        }

        TypeDesc* type = create<T>();
        TypeRegister::insert(type);

        return type;
    }

    template<typename T>
    TypeFlags type::flags()
    {
        return  (TypeFlag_IS_POINTER    * std::is_pointer_v<T>)
              | (TypeFlag_IS_CONST      * std::is_const_v<T>)
              | (TypeFlag_IS_MEMBER_PTR * std::is_member_pointer_v<T>)
              | (TypeFlag_IS_CLASS      * std::is_class_v<T>);
    }

    template<typename T>
    TypeDesc* type::create(const char* _name)
    {
        // TODO: it would be interesting to have a ClassDesc* type::create(...) version using SFINAE.
        if constexpr ( std::is_class_v<T> )
            return new ClassDesc(type::id<T>(), type::primitive_id<T>(), _name,  type::compiler_name<T>(), type::flags<T>() );
        return new TypeDesc(type::id<T>(), type::primitive_id<T>(), _name,  type::compiler_name<T>(), type::flags<T>() );
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
        const ClassDesc* source_type         = source_ptr->get_class();
        const TypeDesc*  possibly_base_class = type::get<PossiblyBaseClass>();
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