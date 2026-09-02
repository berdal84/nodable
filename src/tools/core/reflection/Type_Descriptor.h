#pragma once

#include <unordered_set>
#include <unordered_map>
#include "bdc/Types.hpp"
#include "bdc/String.hpp"
#include "bdc/Array.hpp"
#include <typeinfo>
#include <vector>
#include <tuple>

#include "Function_Traits.h"
#include "Type_Register.h"

// add this macro to a class declaration to enable reflection on it.
// Must have a parent class having REFLECT_BASE_CLASS macro.
#define DECLARE_REFLECT_EX( VIRTUAL, OVERRIDE ) \
    VIRTUAL const ::tools::Type_Descriptor* get_class() const OVERRIDE \
    { return ::tools::type_get(this); }

#define DECLARE_REFLECT          DECLARE_REFLECT_EX(        ,          )
#define DECLARE_REFLECT_virtual  DECLARE_REFLECT_EX( virtual,          )
#define DECLARE_REFLECT_override DECLARE_REFLECT_EX(        , override )

namespace tools
{
    // few empty struct to use as place holder
    struct any{};       // Any type (like TypeScript's)
    struct unknown{};   // Unknown type (like TypeScript's)
    struct null{};      // Absence of type

    typedef int Type_Flags;
    enum Type_Flags_ : int
    {
        Type_Flags_NONE              = 0,
        Type_Flags_IS_CONST          = 1 << 1,
        Type_Flags_IS_POINTER        = 1 << 2,
        Type_Flags_IS_MEMBER_PTR     = 1 << 3,
        Type_Flags_HAS_PARENT        = 1 << 4,
        Type_Flags_HAS_CHILD         = 1 << 5,
        Type_Flags_IS_INTEGRAL       = 1 << 6,
        Type_Flags_IS_FLOATING_POINT = 1 << 7,
        Type_Flags_IS_CLASS          = 1 << 8,
        Type_Flags_IS_FUNCTION       = 1 << 9,
    };

    //
    // type API
    //
    template<typename T> Type_Descriptor*           type_create(bdc::String name = "" );
    bool                                            type_is_implicitly_convertible(const Type_Descriptor* from, const Type_Descriptor* to);
    bool                                            type_equals(const Type_Descriptor*, const Type_Descriptor*);
    const Type_Descriptor*                          type_any();
    const Type_Descriptor*                          type_null();
    template<typename T> std::type_index            type_get_id();
    template<typename T> std::type_index            type_get_primitive_id();
    template<typename T> const char*                type_get_compiler_name();
    template<typename T> Type_Flags                 type_get_flags();
    template<typename T> Type_Descriptor*           type_get();
    template<typename T> Type_Descriptor*           type_get(T value) { return type_get<T>(); }

    //
    // Simple object to store a named function argument
    //
    struct Function_Arg_Descriptor
    {
        const Type_Descriptor* type;
        bool                   pass_by_ref;
        bdc::String            name;
    };

    /**
     * @class TypeDesc (type descriptor) holds meta data relative to a given type.
     *
     * @example @code
     * const TypeDesc* t = type_get<int>();
     * ASSERT( t->is_ptr() == false );
     */
    struct Type_Descriptor
    {
        bdc::String         name          = "";
        bdc::String         compiler_name = "";
        Type_Flags          flags         = Type_Flags_NONE;
        std::type_index     primitive_id  = std::type_index(typeid(null)); // ex: int
        std::type_index     id            = std::type_index(typeid(null)); // ex: int**, int*

        struct Function
        {
            bdc::Inlined_Array<Function_Arg_Descriptor, 8> args;
            const Type_Descriptor*                         return_type;
        };

        struct Class
        {
            std::unordered_set<std::type_index> parents;
            std::unordered_set<std::type_index> children;
        };

        union
        {
            Function function;
            Class    clss;
        };

        Type_Descriptor() {}
        ~Type_Descriptor()
        {
                 if( flags & Type_Flags_IS_CLASS)    clss.~Class();
            else if( flags & Type_Flags_IS_FUNCTION) function.~Function();
        }

        Type_Descriptor(const Type_Descriptor& other)
        {
            #warning not implemented, handle the tagged union 
        }

        Type_Descriptor& operator=(const Type_Descriptor& other)
        {
            #warning not implemented, handle the tagged union
            return *this;
        }

        bool                      is_class() const { return flags & Type_Flags_IS_CLASS; }
        bool                      any_of(std::vector<const Type_Descriptor*> args)const;
        bool                      has_parent() const { return flags & Type_Flags_HAS_PARENT; }
        bool                      is_ptr() const { return flags & Type_Flags_IS_POINTER; }
        bool                      is_const() const { return flags & Type_Flags_IS_CONST; }
        bool                      is_integer() const { return flags & Type_Flags_IS_INTEGRAL; }
        bool                      is_floating_point() const { return flags & Type_Flags_IS_FLOATING_POINT; }
        bool                      equals(const Type_Descriptor* other) const { return type_equals(this, other); }
        template<typename T> bool is() const;
        bool                      is_implicitly_convertible(const Type_Descriptor* _dst ) const;


        template<typename T>                          void function_init(const bdc::String& name);
        template<int ARG_INDEX, typename ArgsAsTuple> void function_push_nth_arg();
        template<typename ...Args>                    void function_push_args();

        bool                        function_is_exactly(const Type_Descriptor*)const;
        bool                        function_is_compatible(const Type_Descriptor*)const;
        void                        function_push_arg(const Type_Descriptor* _type, bool _pass_by_ref = false);
        bool                        function_has_arg_with_type(const Type_Descriptor*)const;

        void                        class_add_parent(std::type_index _parent);
        void                        class_add_child(std::type_index _child);
        bool                        class_is_child_of(std::type_index _possible_parent_id, bool _selfCheck = true) const;
        template<class T> bool      class_is_child_of() const { return class_is_child_of(std::type_index(typeid(T)), true); }
        template<class T> bool      class_is_not_child_of() const { return !class_is_child_of<T>(); }

    };
    
    // Return true if T is reflected
    template<class T, typename GET_CLASS = decltype(&T::get_class)>
    constexpr bool Is_Reflected_Class = std::is_member_function_pointer_v<GET_CLASS>;

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
        inline constexpr static const char* name() { return typeid(type).name(); };
    };

    template<typename T>
    std::type_index type_get_id()
    { return std::type_index(typeid(T)); }

    template<typename T>
    std::type_index type_get_primitive_id()
    { return type_get_id<typename remove_pointer<T>::type>(); }

    template<typename T>
    const char* type_get_compiler_name()
    { return typeid(T).name(); }

    template<int N, typename ArgsAsTuple>
    void Type_Descriptor::function_push_nth_arg()
    {
        using NTH_ARG = std::tuple_element_t<N, ArgsAsTuple>;
        this->function_push_arg( type_get<NTH_ARG>(), std::is_reference_v<NTH_ARG> );
    }

    template<typename ...Args>
    void Type_Descriptor::function_push_args()
    {
        constexpr size_t ARG_COUNT = std::tuple_size_v<Args...>;
        static_assert(ARG_COUNT <= 8, "maximum 8 arguments can be pushed at once");

        // note: I duplicate instead of using template recursion hell. :)

        if constexpr (ARG_COUNT > 0 ) this->function_push_nth_arg<0, Args...>();
        if constexpr (ARG_COUNT > 1 ) this->function_push_nth_arg<1, Args...>();
        if constexpr (ARG_COUNT > 2 ) this->function_push_nth_arg<2, Args...>();
        if constexpr (ARG_COUNT > 3 ) this->function_push_nth_arg<3, Args...>();
        if constexpr (ARG_COUNT > 4 ) this->function_push_nth_arg<4, Args...>();
        if constexpr (ARG_COUNT > 5 ) this->function_push_nth_arg<5, Args...>();
        if constexpr (ARG_COUNT > 6 ) this->function_push_nth_arg<6, Args...>();
        if constexpr (ARG_COUNT > 7 ) this->function_push_nth_arg<7, Args...>();
    }

    template<typename T>
    bool Type_Descriptor::is() const
    { return type_equals(this, type_get<T>()); }

    template<typename T>
    Type_Descriptor* type_create(const char* name)
    {
        Type_Descriptor* type = new Type_Descriptor();
        type_init<T>(type, name);

        return type;
    }

    template<typename T>
    void type_init(Type_Descriptor* type)
    {
        if constexpr ( std::is_class_v<T> )
        {
            new (&type->clss) Type_Descriptor::Class();

            type->id            = type_get_id<T>();
            type->primitive_id  = type_get_primitive_id<T>();
            type->compiler_name = type_get_compiler_name<T>();
            type->flags         = type_get_flags<T>();
        }
        else if constexpr ( std::is_function_v<T> )
        {
            new (&type->function) Type_Descriptor::Function();

            type->id            = type_get_id<T>();
            type->primitive_id  = type_get_primitive_id<T>();
            type->compiler_name = type_get_compiler_name<T>();
            type->flags         = type_get_flags<T>();
            
            type->function.return_type   = type_get<typename Function_Trait<T>::Result_Type >();

            using Args_Type = typename Function_Trait<T>::Args_Type;
            if constexpr ( std::tuple_size_v<Args_Type> != 0)
            {
                type->function_push_args<Args_Type>();
            }
        }
        else
        {
            type->id            = type_get_id<T>();
            type->primitive_id  = type_get_primitive_id<T>();
            type->compiler_name = type_get_compiler_name<T>();
            type->flags         = type_get_flags<T>();
        }
    }

    template<typename T>
    void type_init(Type_Descriptor* type, const char* name)
    {
        type_init<T>(type);
        type->name = name;
    }

    template<typename T>
    Type_Descriptor* type_get()
    {
        auto id = type_get_id<T>();

        if ( type_register_has(id) )
        {
            return type_register_get(id);
        }

        Type_Descriptor* descriptor = new Type_Descriptor();
        type_init<T>(descriptor);
        type_register_insert(descriptor);

        return descriptor;
    }

    static_assert( std::is_function_v<void()>);

    template<typename T>
    Type_Flags type_get_flags()
    {
        return  (Type_Flags_IS_POINTER          * std::is_pointer_v<T>)
              | (Type_Flags_IS_CONST            * std::is_const_v<T>)
              | (Type_Flags_IS_MEMBER_PTR       * std::is_member_pointer_v<T>)
              | (Type_Flags_IS_INTEGRAL         * std::is_integral_v<T>)
              | (Type_Flags_IS_FLOATING_POINT   * std::is_floating_point_v<T>)
              | (Type_Flags_IS_CLASS            * std::is_class_v<T>)
              | (Type_Flags_IS_FUNCTION         * (std::is_member_function_pointer_v<T> || std::is_function_v<T>));
    }

    /**
     * Return if Type extends Possibly_Base_Type
     */
    template<class Possibly_Base_Type, class Type, bool self_check = true>
    bool extends(Type* source_ptr)
    {
        // ensure both classes are reflected
        static_assert(Is_Reflected_Class<Type>);
        static_assert(Is_Reflected_Class<Possibly_Base_Type>);

        // check if source_type is a child of possibly_base_class
        const Type_Descriptor*  source_type         = source_ptr->get_class();
        const Type_Descriptor*  possibly_base_class = type_get<Possibly_Base_Type>();
        return source_type->class_is_child_of( possibly_base_class->id, self_check );
    }

    template<class Target_Type>
    Target_Type* cast(Target_Type* source_ptr)
    { return source_ptr; }

    template<class Target_Type, class Type>
    Target_Type* cast(Type* source_ptr)
    {
        static_assert(!std::is_same_v<Target_Type, Type>);

        if( extends<Target_Type>(source_ptr) )
        {
            return static_cast<Target_Type*>(source_ptr);
        }
        return nullptr;
    }

    template<class T, class>
    T* cast(T* ptr)
    {
        return ptr;
    }
}