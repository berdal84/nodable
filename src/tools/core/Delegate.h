#pragma once
#include <functional>
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
            DELEGATE_TYPE_STATIC = 1,
            DELEGATE_TYPE_METHOD = 2
        };

        using Static_Caller_Type = Result_Type(*)(Args_Type...);
        using Method_Caller_Type = Result_Type(*)(void*, Args_Type...);

        Delegate()
        : _m_static_function_ptr(&_null_function)
        , _m_type(DELEGATE_TYPE_STATIC)
        {}

        Delegate(Result_Type(*func)(Args_Type...)) // static/global functions are easy to handle, we add a constructor.
        : _m_static_function_ptr(func)
        , _m_type(DELEGATE_TYPE_STATIC)
        {
            ASSERT( func != nullptr );
        }

        bool is_null() const
        {
            switch (_m_type)
            {
                case DELEGATE_TYPE_STATIC:
                    return _m_static_function_ptr == &_null_function;
                case DELEGATE_TYPE_METHOD:
                    return _m_method.object_ptr == nullptr || _m_method.function_ptr == nullptr;
            }
        }

        const void* object_ptr() const
        {
            ASSERT(_m_type == DELEGATE_TYPE_METHOD );
            return _m_method.object_ptr;
        }
        
        bool callable() const
        {
            if (_m_type == DELEGATE_TYPE_METHOD)
                return _m_method.object_ptr != nullptr
                    && _m_method.function_ptr != nullptr;

            return true; // a static is always callable, it will be &_null_function or any user defined value.
        }

        void bind(void* object_ptr)
        {
            ASSERT( _m_type == DELEGATE_TYPE_METHOD );
            _m_method.object_ptr = object_ptr;
        }

        Result_Type call(Args_Type... args) const
        {
            switch ( _m_type)
            {
                case DELEGATE_TYPE_STATIC:
                    return _m_static_function_ptr(args...);
                case DELEGATE_TYPE_METHOD:
                    return (*_m_method.function_ptr)(_m_method.object_ptr, args...);
            }
        }

        bool operator==(const Delegate& other) const
        {
            if (this->_m_type != other._m_type)
                return false;

            switch ( this->_m_type )
            {
                case DELEGATE_TYPE_STATIC:
                    return this->_m_static_function_ptr == other._m_static_function_ptr;
                case DELEGATE_TYPE_METHOD:
                    return  _m_method.object_ptr   == other._m_method.object_ptr &&
                            _m_method.function_ptr == other._m_method.function_ptr;

            }

        }

        template<auto Method_Type>
        requires std::is_member_function_pointer_v<decltype(Method_Type)>
        static Delegate from_method(void* object_ptr)
        {
            using Class_Type = typename Function_Trait<decltype(Method_Type)>::Class_Type;
            Delegate delegate;
            delegate._m_type = DELEGATE_TYPE_METHOD;
            delegate._m_method.object_ptr   = object_ptr;
            delegate._m_method.function_ptr = &_method_caller<Class_Type, Method_Type>; // <-- get address of a static function able to call the method
            return delegate;
        }

    private:
        Type _m_type; // We could avoid this, but at some brain damage cost due to "template hell".

        union {
            Static_Caller_Type _m_static_function_ptr;
            struct {
                void*         object_ptr;
                Method_Caller_Type function_ptr;
            } _m_method;
        };

        // Can convert a methods to a regular static function with 1arg for the object ptr
        static Result_Type _null_function(Args_Type... args)
        {
            if constexpr ( !std::is_void_v<Result_Type>) {
                return {};
            }
            return;
        }

        // Can convert a methods to a regular static function with 1arg for the object ptr
        template <class TClass,  Result_Type(TClass::*Method_Type)(Args_Type...)>
        static Result_Type _method_caller(void* ptr, Args_Type... args)
        {
            TClass* object_ptr = static_cast<TClass*>(ptr);
            return (object_ptr->*Method_Type)(args...); // The trick is here, the method IS A TYPE!
        }
    };

    // define few alias
    using Simple_Delegate = Delegate<void()>;
}