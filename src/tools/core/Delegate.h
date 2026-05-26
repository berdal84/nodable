#pragma once
#include <functional>
#include <type_traits>
#include "tools/core/assertions.h"
#include "tools/core/reflection/FunctionTraits.h"

namespace tools
{
    template<typename FunctionT>
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
    template<typename R, typename ...Args>
    struct Delegate<R(Args...)>
    {
        enum Type {
            DELEGATE_TYPE_STATIC = 1,
            DELEGATE_TYPE_METHOD = 2
        };

        using StaticCallerT = R(*)(Args...);
        using MethodCallerT = R(*)(void*, Args...);

        Delegate()
        : _m_static_function_ptr(&_null_function)
        , _m_type(DELEGATE_TYPE_STATIC)
        {}

        Delegate(R(*func)(Args...)) // static/global functions are easy to handle, we add a constructor.
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

        R call(Args... args) const
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

        template<auto TMethod>
        requires std::is_member_function_pointer_v<decltype(TMethod)>
        static Delegate from_method(void* object_ptr)
        {
            using T = typename FunctionTrait<decltype(TMethod)>::class_t;
            Delegate delegate;
            delegate._m_type = DELEGATE_TYPE_METHOD;
            delegate._m_method.object_ptr   = object_ptr;
            delegate._m_method.function_ptr = &_method_caller<T, TMethod>; // <-- get address of a static function able to call the method
            return delegate;
        }

    private:
        Type _m_type; // We could avoid this, but at some brain damage cost due to "template hell".

        union {
            StaticCallerT _m_static_function_ptr;
            struct {
                void*         object_ptr;
                MethodCallerT function_ptr;
            } _m_method;
        };

        // Can convert a methods to a regular static function with 1arg for the object ptr
        static R _null_function(Args... args)
        {
            if constexpr ( !std::is_void_v<R>) {
                return {};
            }
            return;
        }

        // Can convert a methods to a regular static function with 1arg for the object ptr
        template <class TClass,  R(TClass::*TMethod)(Args...)>
        static R _method_caller(void* ptr, Args... args)
        {
            TClass* object_ptr = static_cast<TClass*>(ptr);
            return (object_ptr->*TMethod)(args...); // The trick is here, the method IS A TYPE!
        }
    };

    // define few alias
    using SimpleDelegate = Delegate<void()>;
}