#pragma once
#include <functional>
#include "tools/core/reflection/function_traits.h"

namespace tools
{
    // TODO: avoid to use std::function
    template<typename ...Args>
    struct DelegateT
    {
        typedef std::function<void(void*, Args...)> F;
        F m_function;

        DelegateT()
        : m_function([](void*) {})
        {}

        DelegateT(F&& function )
        : m_function(std::move(function))
        {}

        void operator()(void* object_ptr, Args... args) const
        {  m_function(object_ptr, args...); }
    };

    // Very simple delegate, works only on members taking 0 arguments
    typedef DelegateT<> Delegate_NoArgs;

    struct DelegateFactory
    {
        template<typename R, typename T, typename ...Args>
        static DelegateT<Args...>
        create( R(T::*member_ptr)(Args...) )
        {
            auto function = [member_ptr](void* object_ptr, Args... args)
            {
                return (((T*)object_ptr)->*member_ptr)(args...);
            };

            return {function};
        }
    };
}