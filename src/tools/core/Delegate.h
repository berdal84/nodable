#pragma once
#include <tuple>
#include <type_traits>
#include "tools/core/Asserts.h"
#include "tools/core/reflection/Function_Traits.h"

namespace tools
{
    template<typename Function_Type>
    struct Delegate;

    //
    // struct Delegate is able to wrap a method and call it later on a given object_ptr
    // It does not rely on std::function, and is made from this code https://www.codeproject.com/Articles/11015/The-Impossibly-Fast-C-Delegates
    //
    // ex:
    // Delegate<void> d = Delegate<void>::from_method<&MyClass::my_method>( my_class_instance_ptr );
    // d.call();
    //
    // or
    //
    // Delegate<void> d = Delegate<void>::from_method<&MyClass::my_method>();
    // b.bind(my_class_instance_ptr);
    // d.call();
    //
    template<typename Result_Type, typename ...Args_Type>
    struct Delegate<Result_Type(Args_Type...)>
    {
        enum Type {
            DELEGATE_TYPE_NONE   = 0,
            DELEGATE_TYPE_STATIC = 1,
            DELEGATE_TYPE_METHOD = 2
        };

        using Static_Caller_Type = Result_Type(*)(Args_Type...);
        using Method_Caller_Type = Result_Type(*)(void*, Args_Type...);

        Delegate() = default;

        Delegate(Result_Type(*func)(Args_Type...)) // static/global functions are easy to handle, we add a constructor.
        : static_function_ptr(func)
        , type(DELEGATE_TYPE_STATIC)
        {
            ASSERT( func != nullptr );
        }

        bool is_null() const
        {
            switch (type)
            {
                case DELEGATE_TYPE_NONE:
                    return true;
                case DELEGATE_TYPE_STATIC:
                    return static_function_ptr == &_null_function;
                case DELEGATE_TYPE_METHOD:
                    return object_ptr == nullptr || method_function_ptr == nullptr;
            }
        }

        bool callable() const
        {
            if (type == DELEGATE_TYPE_METHOD)
                return object_ptr != nullptr
                    && method_function_ptr != nullptr;

            return true; // a static is always callable, it will be &_null_function or any user defined value.
        }

        void bind(void* new_object_ptr)
        {
            ASSERT( type == DELEGATE_TYPE_METHOD );
            object_ptr = new_object_ptr;
        }

        Result_Type call(Args_Type... args) const
        {
            switch ( type)
            {
                case DELEGATE_TYPE_STATIC:
                    return static_function_ptr(args...);
                case DELEGATE_TYPE_METHOD:
                    return (*method_function_ptr)(object_ptr, args...);
                case DELEGATE_TYPE_NONE:
                    return;
            }
        }

        bool operator==(const Delegate& other) const
        {
            if (this->type != other.type)
                return false;

            switch ( this->type )
            {
                case DELEGATE_TYPE_STATIC:
                    return this->static_function_ptr == other.static_function_ptr;
                case DELEGATE_TYPE_METHOD:
                    return  object_ptr == other.object_ptr &&
                            method_function_ptr == other.method_function_ptr;
                case DELEGATE_TYPE_NONE:
                    return true;
            }

        }

       
        template<auto Function>
        static Delegate from(void* object_ptr = nullptr /* we allow to call bind() later on. */)
        {
            Delegate delegate;
            delegate.type       = DELEGATE_TYPE_METHOD; // we consider c-style functions as "methods".
            delegate.object_ptr = object_ptr;

            if constexpr (std::is_member_function_pointer_v<decltype(Function)>)
            {
                using Class_Type = typename Function_Trait<decltype(Function)>::Class_Type;
                delegate.method_function_ptr = &_method_caller<Class_Type, Function>;
            }
            else
            {
                using Struct_Type = typename Function_Trait<decltype(Function)>::First_Arg_Type ;
                delegate.method_function_ptr = &_cstyle_method_caller<Struct_Type, Function>;
            }
            
            return delegate;
        }

        Type                type                = DELEGATE_TYPE_NONE; // We could avoid this, but at some brain damage cost due to "template hell".
        Static_Caller_Type  static_function_ptr = nullptr;
        Method_Caller_Type  method_function_ptr = nullptr;
        void*               object_ptr          = nullptr;

        // Can convert a methods to a regular static function with 1arg for the object ptr
        static Result_Type _null_function(Args_Type... args)
        {
            if constexpr ( !std::is_void_v<Result_Type>) {
                return {};
            }
            return;
        }

        // Can convert a methods to a regular static function with 1arg for the object ptr
        template <class TClass,  Result_Type(TClass::*Method)(Args_Type...)>
        static Result_Type _method_caller(void* ptr, Args_Type... args)
        {
            TClass* object_ptr = static_cast<TClass*>(ptr);
            return (object_ptr->*Method)(args...); // The trick is here, the method IS A TYPE!
        }

        template <typename First_Arg_Type,  Result_Type(*CStyle_Method)(First_Arg_Type, Args_Type...)>
        static Result_Type _cstyle_method_caller(void* ptr, Args_Type... args)
        {
            auto* data_ptr = static_cast<First_Arg_Type>(ptr);
            return (*CStyle_Method)(data_ptr, args...);
        }
    };

    // define few alias
    using Simple_Delegate = Delegate<void()>;
}