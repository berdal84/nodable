#pragma once
#include <functional>
#include "tools/core/assertions.h"
#include "tools/core/reflection/FunctionTraits.h"

namespace tools
{
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
    struct Delegate
    {
        using TFunction = R(void*, Args...);

        void*      _object_ptr        = nullptr;
        TFunction* _method_caller_ptr = &_method_caller_null;

        template<auto TMethod, class T = typename FunctionTrait<decltype(TMethod)>::class_t>
        static Delegate<R, Args...> from_method(void* object_ptr = nullptr)
        {
            Delegate<R, Args...> d{};
            d._object_ptr   = object_ptr;
            d._method_caller_ptr = &_method_caller<T, TMethod>; // <-- get address of a static function able to call the method
            return d;
        }

        inline void bind(void* object_ptr)
        { _object_ptr = object_ptr; }

        inline R call(Args... args) const
        { return (*_method_caller_ptr)(_object_ptr, args...); }

        // To act as a null method called
        inline static R _method_caller_null(void* object_ptr, Args... args)
        {
            if constexpr ( !std::is_void_v<R> )
                return {};
            return;
        }

        template <class T,  R(T::*TMethod)(Args...)>
        inline static R _method_caller(void* object_ptr, Args... args)
        {
            T* p = static_cast<T*>(object_ptr);
            VERIFY(p != nullptr, "object_ptr is null, did you provided it?");
            return (p->*TMethod)(args...); // The trick is here, the method IS A TYPE!
        }
    };

    // define few alias
    typedef Delegate<void> Delegate_NoArgs;
}