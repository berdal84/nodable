#pragma once

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <typeinfo>
#include <vector>
#include <tuple>

#include "Function_Traits.h"
#include "Type_Register.h"

// add this macro to a class declaration to enable reflection on it.
// Must have a parent class having REFLECT_BASE_CLASS macro.
#define DECLARE_REFLECT_EX( VIRTUAL, OVERRIDE ) \
    VIRTUAL const ::tools::Class_Descriptor* get_class() const OVERRIDE \
    { return ::tools::type::get_class(this); }

#define DECLARE_REFLECT          DECLARE_REFLECT_EX(        ,          )
#define DECLARE_REFLECT_virtual  DECLARE_REFLECT_EX( virtual,          )
#define DECLARE_REFLECT_override DECLARE_REFLECT_EX(        , override )

namespace tools
{
    // forward declarations
    class Function_Descriptor;
    class IInvokable;
    class IInvokable_Method;
    class Type_Descriptor;
    class Class_Descriptor;
    class Operator;

    struct any{};  // Any type (like TypeScript's)
    struct unknown{}; // Unknown type (like TypeScript's)
    struct null{}; // Absence of type

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

    typedef int TypeFlags;
    enum TypeFlag_ : int
    {
        TypeFlag_NONE              = 0,
        TypeFlag_IS_CLASS          = 1 << 0,
        TypeFlag_IS_CONST          = 1 << 1,
        TypeFlag_IS_POINTER        = 1 << 2,
        TypeFlag_IS_MEMBER_PTR     = 1 << 3,
        TypeFlag_HAS_PARENT        = 1 << 5,
        TypeFlag_HAS_CHILD         = 1 << 6,
        TypeFlag_IS_INTEGRAL       = 1 << 7,
        TypeFlag_IS_FLOATING_POINT = 1 << 8
    };

    // Type utilities
    namespace type
    {
        bool               is_implicitly_convertible(const Type_Descriptor* _src, const Type_Descriptor* _dst);
        bool               equals(const Type_Descriptor* left, const Type_Descriptor* right);
        const Type_Descriptor*    any();
        const Type_Descriptor*    null();

        template<typename T> std::type_index    get_id();
        template<typename T> std::type_index    get_primitive_id();
        template<typename T> const char*        get_compiler_name();
        template<typename T> TypeFlags          get_flags();
        template<typename T> const Type_Descriptor*    get();
        template<typename T> const Class_Descriptor*   get_class(T* ptr);
        template<typename T> const Class_Descriptor*   get_class();
        template<typename T> Type_Descriptor*          create(const char* _name = "");
        template<typename T> const Type_Descriptor*    get(T value) { return get<T>(); }
    };

    /**
     * @class TypeDesc (type descriptor) holds meta data relative to a given type.
     *
     * @example @code
     * const TypeDesc* t = type::get<int>();
     * assert( t->is_ptr() == false );
     */
    class Type_Descriptor
    {
        friend Type_Register;
    public:
        Type_Descriptor()
        : m_id(std::type_index(typeid(null))), m_primitive_id( std::type_index(typeid(null)) ) {}
        Type_Descriptor(std::type_index _id, std::type_index _primitive_id)
        : m_id(_id), m_primitive_id(_primitive_id) {}

        virtual ~Type_Descriptor() {};

        template<class T>
        static Type_Descriptor* create(const char* _name);
        std::type_index           id() const { return m_id; }
        const char*               compiler_name() const { return m_compiler_name; };
        const char*               name() const { return m_name.c_str(); };
        bool                      is_class() const { return m_flags & TypeFlag_IS_CLASS; }
        bool                      any_of(std::vector<const Type_Descriptor*> args)const;
        bool                      has_parent() const { return m_flags & TypeFlag_HAS_PARENT; }
        bool                      is_ptr() const { return m_flags & TypeFlag_IS_POINTER; }
        bool                      is_const() const { return m_flags & TypeFlag_IS_CONST; }
        bool                      is_integer() const { return m_flags & TypeFlag_IS_INTEGRAL; }
        bool                      is_floating_point() const { return m_flags & TypeFlag_IS_FLOATING_POINT; }
        bool                      equals(const Type_Descriptor* other) const { return type::equals(this, other); }
        template<typename T>
        bool                      is() const;
        bool                      is_implicitly_convertible(const Type_Descriptor* _dst ) const;
    protected:
        std::string m_name;
        const char* m_compiler_name = nullptr;
        TypeFlags   m_flags         = TypeFlag_NONE;
        std::type_index m_primitive_id; // ex: int
        std::type_index m_id;           // ex: int**, int*
    };

    /*
     * Simple object to store a named function argument
     */
    struct Function_Arg_Descriptor
    {
        const Type_Descriptor* type;
        bool                   pass_by_ref;
        std::string            name;
    };

    /*
     * Class to store a function signature.
     * We can check if two function signature are matching using this->match(other)
     */
    class Function_Descriptor : public Type_Descriptor
    {
    public:

        template<typename T> static Function_Descriptor* create(const char* _name);

        Function_Descriptor() = default;
        bool                           is_exactly(const Function_Descriptor*)const;
        bool                           is_compatible(const Function_Descriptor*)const;
        const char*                    get_identifier()const { return m_name.c_str(); };
        Function_Arg_Descriptor&                       arg_at(size_t i) { return m_argument[i]; }
        const Function_Arg_Descriptor&                 arg_at(size_t i) const { return m_argument[i]; }
        std::vector<Function_Arg_Descriptor>&          arg() { return m_argument;};
        const std::vector<Function_Arg_Descriptor>&    arg()const { return m_argument;};
        size_t                         arg_count() const { return m_argument.size(); }
        const Type_Descriptor*          return_type() const { return m_return_type; }
        void                           set_return_type(const Type_Descriptor* _type) { m_return_type = _type; };
        template<typename T> void init(const char* _name);
        template<int ARG_INDEX, typename ArgsAsTuple>
        void                           push_nth_arg();
        template<typename ...Args>
        void                           push_args();
        void                           push_arg(const Type_Descriptor* _type, bool _pass_by_ref = false);
        bool                           has_arg_with_type(const Type_Descriptor*)const;
    private:
        std::vector<Function_Arg_Descriptor>   m_argument;
        const Type_Descriptor*  m_return_type = type::null();
    };

    /**
     * @class ClassDesc (class descriptor) holds meta data relative to a given class.
     *
     * @example @code
     * const TypeDesc* class_desc = type::get<std::string>();
     * assert( class_desc->is_class() );
     */
    class Class_Descriptor : public Type_Descriptor
    {
        friend Type_Register;
    public:
        Class_Descriptor() = default;
        ~Class_Descriptor();

        template<class T>
        static Class_Descriptor* create(const char* _name);

        void                      add_parent(std::type_index _parent);
        void                      add_child(std::type_index _child);
        void                      add_static(const char* _name, const IInvokable*);
        void                      add_method(const char* _name, const IInvokable_Method*);
        const std::unordered_set<const IInvokable*>&       get_statics()const { return m_static_methods; }
        const std::unordered_set<const IInvokable_Method*>& get_methods()const { return m_methods; }
        const IInvokable*         get_static(const char* _name) const;
        const IInvokable_Method*   get_method(const char* _name) const;
        bool                      is_child_of(std::type_index _possible_parent_id, bool _selfCheck = true) const;
        template<class T>
        bool               is_child_of() const { return is_child_of(std::type_index(typeid(T)), true); }
        template<class T>
        bool               is_not_child_of() const { return !is_child_of<T>(); }

    protected:
        std::unordered_set<std::type_index>                      m_parents;
        std::unordered_set<std::type_index>                      m_children;
        std::unordered_set<const IInvokable*>                    m_static_methods;
        std::unordered_map<std::string, const IInvokable*>       m_static_methods_by_name;
        std::unordered_set<const IInvokable_Method*>              m_methods;
        std::unordered_map<std::string, const IInvokable_Method*> m_methods_by_name;
    };

    template<typename T>
    std::type_index type::get_id()
    { return std::type_index(typeid(T)); }

    template<typename T>
    std::type_index type::get_primitive_id()
    { return type::get_id<typename remove_pointer<T>::type>(); }

    template<typename T>
    const char* type::get_compiler_name()
    { return typeid(T).name(); }

    template<int N, typename ArgsAsTuple>
    void Function_Descriptor::push_nth_arg()
    {
        using NTH_ARG = std::tuple_element_t<N, ArgsAsTuple>;
        push_arg( type::get<NTH_ARG>(), std::is_reference_v<NTH_ARG> );
    }

    template<typename ...Args>
    void Function_Descriptor::push_args()
    {
        constexpr size_t ARG_COUNT = std::tuple_size_v<Args...>;
        static_assert(ARG_COUNT <= 8, "maximum 8 arguments can be pushed at once");

        // note: I duplicate instead of using template recursion hell. :)

        if constexpr (ARG_COUNT > 0 ) push_nth_arg<0, Args...>();
        if constexpr (ARG_COUNT > 1 ) push_nth_arg<1, Args...>();
        if constexpr (ARG_COUNT > 2 ) push_nth_arg<2, Args...>();
        if constexpr (ARG_COUNT > 3 ) push_nth_arg<3, Args...>();
        if constexpr (ARG_COUNT > 4 ) push_nth_arg<4, Args...>();
        if constexpr (ARG_COUNT > 5 ) push_nth_arg<5, Args...>();
        if constexpr (ARG_COUNT > 6 ) push_nth_arg<6, Args...>();
        if constexpr (ARG_COUNT > 7 ) push_nth_arg<7, Args...>();
    }

    template<typename T>
    bool Type_Descriptor::is() const
    { return type::equals(this, type::get<T>()); }

    template<typename T>
    Type_Descriptor* Type_Descriptor::create(const char* _name)
    {
        static_assert( std::is_class_v<T> == false );

        Type_Descriptor* descriptor = new Type_Descriptor(type::get_id<T>(), type::get_primitive_id<T>() );

        descriptor->m_name          = _name;
        descriptor->m_compiler_name = type::get_compiler_name<T>();
        descriptor->m_flags         = type::get_flags<T>();

        return descriptor;
    }

    template<typename T>
    Class_Descriptor* Class_Descriptor::create(const char* _name)
    {
        static_assert( std::is_class_v<T> );

        Class_Descriptor* class_desc = new Class_Descriptor();

        class_desc->m_id            = type::get_id<T>();
        class_desc->m_primitive_id  = type::get_primitive_id<T>();
        class_desc->m_name          = _name;
        class_desc->m_compiler_name = type::get_compiler_name<T>();
        class_desc->m_flags         = type::get_flags<T>();

        return class_desc;
    }

    template<typename T>
    Function_Descriptor* Function_Descriptor::create(const char* _name)
    {
        Function_Descriptor* descriptor = new Function_Descriptor();
        descriptor->init<T>(_name);
        return descriptor;
    }

    template<typename T>
    void Function_Descriptor::init(const char* _name)
    {
        m_id            = type::get_id<T>();
        m_primitive_id  = type::get_primitive_id<T>();
        m_compiler_name = type::get_compiler_name<T>();
        m_flags         = type::get_flags<T>();
        m_return_type   = type::get<typename Function_Trait<T>::Result_Type >();
        m_name          = _name;

        using Args_Type = typename Function_Trait<T>::Args_Type;
        if constexpr ( std::tuple_size_v<Args_Type> != 0)
            push_args<Args_Type>();
    }

    template<typename T>
    const Class_Descriptor* type::get_class()
    {
        static_assert( std::is_class_v<T> );
        return (const Class_Descriptor*)get<T>();
    }

    template<typename T>
    const Class_Descriptor* type::get_class(T* ptr)
    {
        static_assert( std::is_class_v<T> );
        return (const Class_Descriptor*)get<T>();
    }

    template<typename T>
    const Type_Descriptor* type::get()
    {
        auto id = type::get_id<T>();

        if ( Type_Register::has(id) )
        {
            return Type_Register::get(id);
        }

        Type_Descriptor* descriptor = type::create<T>();
        Type_Register::insert(descriptor);

        return descriptor;
    }

    template<typename T>
    TypeFlags type::get_flags()
    {
        return  (TypeFlag_IS_POINTER    * std::is_pointer_v<T>)
              | (TypeFlag_IS_CONST      * std::is_const_v<T>)
              | (TypeFlag_IS_MEMBER_PTR * std::is_member_pointer_v<T>)
              | (TypeFlag_IS_INTEGRAL   * std::is_integral_v<T>)
              | (TypeFlag_IS_FLOATING_POINT * std::is_floating_point_v<T>)
              | (TypeFlag_IS_CLASS      * std::is_class_v<T>);
    }

    template<typename T>
    Type_Descriptor* type::create(const char* _name)
    {
        if constexpr ( std::is_member_function_pointer_v<T> || std::is_function_v<T>)
            return Function_Descriptor::create<T>(_name);
        if constexpr ( std::is_class_v<T> )
            return Class_Descriptor::create<T>(_name);
        else
            return Type_Descriptor::create<T>(_name);
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
        const Class_Descriptor* source_type         = source_ptr->get_class();
        const Type_Descriptor*  possibly_base_class = type::get<PossiblyBaseClass>();
        return source_type->is_child_of(possibly_base_class->id(), self_check );
    }

    template<class TargetClass>
    TargetClass* cast(TargetClass* source_ptr)
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
}