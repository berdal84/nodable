#pragma once

#include <functional>
#include <tuple>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <cstddef>

#include <fw/types.h>
#include <fw/reflection/variant.h>
#include <fw/reflection/type.h>
#include <fw/reflection/func_type.h>

namespace fw {

    class iinvokable
    {
    public:
        virtual ~iinvokable() {};
        virtual const func_type& get_type() const = 0;
        virtual variant operator()(const std::vector<variant *> &_args = {}) const = 0;
    };

    class iinvokable_nonstatic
    {
    public:
        virtual ~iinvokable_nonstatic() {};
        virtual const func_type& get_type() const = 0;
        virtual variant operator()(void* _instance, const std::vector<variant *> &_args = {}) const = 0;
    };

#define ENABLE_IF_VOID(T)     typename std::enable_if< std::is_void<T>::value, int>::type = 0
#define ENABLE_IF_NON_VOID(T) typename std::enable_if< !std::is_void<T>::value, int>::type = 0
#define ENABLE_IF_ARGC(count) typename std::enable_if< std::tuple_size<Args>::value == count, int>::type = 0
#define CAST_ARG(index)       (std::tuple_element_t<index, Args>) *_args[index]
#define CAST_1ARG  CAST_ARG(0)
#define CAST_2ARGS CAST_1ARG, CAST_ARG(1)
#define CAST_3ARGS CAST_2ARGS, CAST_ARG(2)
#define CAST_4ARGS CAST_3ARGS, CAST_ARG(3)
#define CAST_5ARGS CAST_4ARGS, CAST_ARG(4)

    /**
     * Helpers to invoke static functions
     */
     // 0 arg
    template<typename T, typename Args, typename F = T(Args...), ENABLE_IF_ARGC(0), ENABLE_IF_NON_VOID(T)>
    variant invoke(F* _function, const std::vector<variant *> &_args)
    {
        return _function();
    }
    template<typename T, typename Args, typename F = T(Args...), ENABLE_IF_ARGC(0), ENABLE_IF_VOID(T)>
    variant invoke(F* _function, const std::vector<variant *> &_args)
    {
        _function();
        return null_t{};
    }

    // 1 arg
    template<typename T, typename Args, typename F = T(Args...),  ENABLE_IF_ARGC(1), ENABLE_IF_NON_VOID(T)>
    variant invoke(F* _function , const std::vector<variant *> &_args)
    {
        return _function(CAST_1ARG);
    }
    template<typename T, typename Args, typename F = T(Args...), ENABLE_IF_ARGC(1), ENABLE_IF_VOID(T)>
    variant invoke(F* _function , const std::vector<variant *> &_args)
    {
        _function(CAST_1ARG);
        return null_t{};
    }
    // 2 args
    template<typename T, typename Args, typename F = T(Args...), ENABLE_IF_ARGC(2), ENABLE_IF_NON_VOID(T)>
    variant invoke(F* _function , const std::vector<variant *> &_args)
    {
        return _function(CAST_2ARGS);
    }
    template<typename T, typename Args, typename F = T(Args...), ENABLE_IF_ARGC(2), ENABLE_IF_VOID(T)>
    variant invoke(F* _function, const std::vector<variant *> &_args)
    {
        _function(CAST_2ARGS);
        return null_t{};
    }
    // 3 args
    template<typename T, typename Args, typename F = T(Args...), ENABLE_IF_ARGC(3), ENABLE_IF_NON_VOID(T)>
    variant invoke(F* _function, const std::vector<variant *> &_args)
    {
        return _function(CAST_3ARGS);
    }
    template<typename T, typename Args, typename F = T(Args...), ENABLE_IF_ARGC(3), ENABLE_IF_VOID(T)>
    variant invoke(F* _function, const std::vector<variant *> &_args)
    {
        _function(CAST_3ARGS);
        return null_t{};
    }

    // 4 args
    template<typename T, typename Args, typename F = T(Args...), ENABLE_IF_ARGC(4), ENABLE_IF_NON_VOID(T)>
    variant invoke(F* _function, const std::vector<variant *> &_args)
    {
        return _function(CAST_4ARGS);
    }
    template<typename T, typename Args, typename F = T(Args...), ENABLE_IF_ARGC(4), ENABLE_IF_VOID(T)>
    variant invoke(F* _function, const std::vector<variant *> &_args)
    {
        _function(CAST_4ARGS);
        return null_t{};
    }

    // 5 args
    template<typename T, typename Args, typename F = T(Args...), ENABLE_IF_ARGC(5), ENABLE_IF_NON_VOID(T)>
    variant invoke(F* _function, const std::vector<variant *> &_args)
    {
        return _function(CAST_5ARGS);
    }
    template<typename T, typename Args, typename F = T(Args...), ENABLE_IF_ARGC(5), ENABLE_IF_VOID(T)>
    variant invoke(F* _function, const std::vector<variant *> &_args)
    {
        _function(CAST_5ARGS);
        return null_t{};
    }

    /**
     * Helpers to invoke property function from an instance pointer
     */
     // no args
     template<typename T, typename C, typename Args, typename F = T(C::*)(Args...), ENABLE_IF_ARGC(0), ENABLE_IF_NON_VOID(T)>
     variant invoke_property(C* _instance, F _function, const std::vector<variant *> &_args)
     {
         return (_instance->*_function)();
     }
    template<typename T, typename C, typename Args, typename F = T(C::*)(Args...), ENABLE_IF_ARGC(0), ENABLE_IF_VOID(T)>
    variant invoke_property(C* _instance, F _function, const std::vector<variant *> &_args)
    {
        (_instance->*_function)();
        return null_t{};
    }
    // 1 arg
    template<typename T, typename C, typename Args, typename F = T(C::*)(Args...), ENABLE_IF_ARGC(1), ENABLE_IF_NON_VOID(T)>
    variant invoke_property(C* _instance, F _function, const std::vector<variant *> &_args)
    {
        return (_instance->*_function)(CAST_1ARG);
    }
    template<typename T, typename C, typename Args, typename F = T(C::*)(Args...), ENABLE_IF_ARGC(1), ENABLE_IF_VOID(T)>
    variant invoke_property(C* _instance, F _function, const std::vector<variant *> &_args)
    {
        (_instance->*_function)(CAST_1ARG);
        return null_t{};
    }
    // 2 args
    template<typename T, typename C, typename Args, typename F = T(C::*)(Args...), ENABLE_IF_ARGC(2), ENABLE_IF_NON_VOID(T)>
    variant invoke_property(C* _instance, F _function, const std::vector<variant *> &_args)
    {
        return (_instance->*_function)(CAST_2ARGS);
    }
    template<typename T, typename C, typename Args, typename F = T(C::*)(Args...), ENABLE_IF_ARGC(2), ENABLE_IF_VOID(T)>
    variant invoke_property(C* _instance, F _function, const std::vector<variant *> &_args)
    {
        (_instance->*_function)(CAST_2ARGS);
        return null_t{};
    }
    // 3 args
    template<typename T, typename C, typename Args, typename F = T(C::*)(Args...), ENABLE_IF_ARGC(3), ENABLE_IF_NON_VOID(T)>
    variant invoke_property(C* _instance, F _function, const std::vector<variant *> &_args)
    {
        return (_instance->*_function)(CAST_3ARGS);
    }
    template<typename T, typename C, typename Args, typename F = T(C::*)(Args...), ENABLE_IF_ARGC(3), ENABLE_IF_VOID(T)>
    variant invoke_property(C* _instance, F _function, const std::vector<variant *> &_args)
    {
        (_instance->*_function)(CAST_3ARGS);
        return null_t{};
    }
    // 4 args
    template<typename T, typename C, typename Args, typename F = T(C::*)(Args...), ENABLE_IF_ARGC(4), ENABLE_IF_NON_VOID(T)>
    variant invoke_property(C* _instance, F _function, const std::vector<variant *> &_args)
    {
        return (_instance->*_function)(CAST_4ARGS);
    }
    template<typename T, typename C, typename Args, typename F = T(C::*)(Args...), ENABLE_IF_ARGC(4), ENABLE_IF_VOID(T)>
    variant invoke_property(C* _instance, F _function, const std::vector<variant *> &_args)
    {
        (_instance->*_function)(CAST_4ARGS);
        return null_t{};
    }
    // 5 args
    template<typename T, typename C, typename Args, typename F = T(C::*)(Args...), ENABLE_IF_ARGC(5), ENABLE_IF_NON_VOID(T)>
    variant invoke_property(C* _instance, F _function, const std::vector<variant *> &_args)
    {
        return (_instance->*_function)(CAST_5ARGS);
    }
    template<typename T, typename C, typename Args, typename F = T(C::*)(Args...), ENABLE_IF_ARGC(5), ENABLE_IF_VOID(T)>
    variant invoke_property(C* _instance, F _function, const std::vector<variant *> &_args)
    {
        (_instance->*_function)(CAST_5ARGS);
        return null_t{};
    }

#undef ENABLE_IF_NON_VOID
#undef ENABLE_IF_VOID
#undef ENABLE_IF_ARGC
#undef CAST_ARG
#undef CAST_1ARG
#undef CAST_2ARGS
#undef CAST_3ARGS
#undef CAST_4ARGS
#undef CAST_5ARGS

    template<typename T>
    class invokable_static;

    /** Generic Invokable (works for static only) */
    template<typename T, typename... Ts>
    class invokable_static<T(Ts...)> : public iinvokable
    {
    public:
        using function_t  = T(Ts...);
        using return_t    = T;
        using args_t      = std::tuple<Ts...>;

        invokable_static(function_t* _implem, const char* _name)
        : m_function_impl(_implem)
        , m_function_type(*func_type_builder<function_t>::with_id(_name))
        {
            NDBL_ASSERT(m_function_impl)
        }

        ~invokable_static() override {}

        variant operator()(const std::vector<variant *> &_args = {}) const override
        {
            NDBL_EXPECT(_args.size() == std::tuple_size<args_t>(), "Wrong argument count!");
            return fw::invoke<return_t, args_t >(m_function_impl, _args);
        }

        const func_type& get_type() const override { return m_function_type; }

    private:
        function_t* const m_function_impl;
        func_type         m_function_type;
    };

    template<typename T>
    class invokable_nonstatic;

    /**
     * wrapper for NON STATIC methods ONLY
     */
    template<typename R, typename C, typename ...Ts>
    class invokable_nonstatic<R(C::*)(Ts...)> : public iinvokable_nonstatic // WIP...
    {
        using return_t   = R;
        using class_t    = C;
        using args_t     = std::tuple<Ts...>;
        using function_t = R(C::*)(Ts...);
        const func_type  m_method_type;
        function_t       m_method;
    public:

        invokable_nonstatic(function_t _method , const char* _name)
        : m_method( _method )
        , m_method_type(*func_type_builder<function_t>::with_id(_name) )
        {
        }

        const func_type& get_type() const override { return m_method_type; };

        virtual variant operator()(void* _instance, const std::vector<variant *> &_args = {}) const override
        {
            NDBL_EXPECT(_args.size() == std::tuple_size<args_t>(), "Wrong argument count!");
            return fw::invoke_property<return_t, class_t, args_t>(reinterpret_cast<class_t *>(_instance), m_method, _args);
        };
    };

}