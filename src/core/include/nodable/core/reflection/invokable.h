#pragma once

#include <functional>
#include <tuple>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <cstddef>

#include <nodable/core/types.h>
#include <nodable/core/reflection/variant.h>
#include <nodable/core/reflection/type.h>
#include <nodable/core/reflection/func_type.h>

namespace Nodable {

    class iinvokable
    {
    public:
        virtual ~iinvokable() {};
        virtual const func_type& get_type() const = 0;
        virtual variant invoke(const std::vector<variant *> &_args = {}) const = 0;
    };

    /** Helpers to call a function (need serious work here) */

    /** 0 arg function */

    // static non-void
    template<
            typename T,
            typename F = T(),
            typename std::enable_if< !std::is_void<T>::value, int>::type = 0>
    variant invoke(F *_function, const std::vector<variant *> &_args)
    {
        return _function();
    }

    // static void
    template<
            typename T,
            typename F = T(),
            typename std::enable_if< std::is_void<T>::value, int>::type = 0>
    variant invoke(F *_function, const std::vector<variant *> &_args)
    {
        _function();
        return null_t{};
    }

    // non-void
    template<typename T, typename C, typename std::enable_if< !std::is_void<T>::value, int>::type = 0>
    variant invoke(C* _instance, T(C::*_function)(), const std::vector<variant *> &_args)
    {
        NODABLE_ASSERT_EX(_args.empty(), "Parameters are noy handled!");
        // TODO: handle parameters
        return (_instance->*_function)();
    }
    // void
    template<typename T, typename C, typename std::enable_if< std::is_void<T>::value, int>::type = 0>
    variant invoke(C* _instance, T(C::*_function)(), const std::vector<variant *> &_args)
    {
        NODABLE_ASSERT_EX(_args.empty(), "Parameters are noy handled!");
        // TODO: handle parameters
        (_instance->*_function)();
        return null_t{};
    }

    /** 1 arg function */
    // non-void
    template<
            typename T,
            typename A0,
            typename F = T(A0),
            typename std::enable_if< !std::is_void<T>::value, int>::type = 0>
    variant invoke(F *_function, const std::vector<variant *> &_args)
    {
        return _function( (A0) *_args[0] );
    }
    // void
    template<
            typename T,
            typename A0,
            typename F = T(A0),
            typename std::enable_if< std::is_void<T>::value, int>::type = 0>
    variant invoke(F *_function, const std::vector<variant *> &_args)
    {
        _function( (A0) *_args[0] );
        return null_t{};
    }

    /** 2 arg function */
    // non-void
    template<
            typename T,
            typename A0,
            typename A1,
            typename F = T(A0, A1),
            typename std::enable_if< !std::is_void<T>::value, int>::type = 0>
    variant invoke(F *_function, const std::vector<variant *> &_args)
    {
        return _function( (A0) *_args[0], (A1) *_args[1] );
    }
    // void
    template<
            typename T,
            typename A0,
            typename A1,
            typename F = T(A0, A1),
            typename std::enable_if< std::is_void<T>::value, int>::type = 0>
    variant invoke(F *_function, const std::vector<variant *> &_args)
    {
        _function( (A0) *_args[0], (A1) *_args[1] );
        return null_t{};
    }

    /** 3 arg function */
    // non-void
    template<
            typename T,
            typename A0,
            typename A1,
            typename A2,
            typename F = T(A0, A1, A2),
            typename std::enable_if< !std::is_void<T>::value, int>::type = 0>
    variant invoke(F *_function, const std::vector<variant *> &_args)
    {
        _function( (A0) *_args[0], (A1) *_args[1], (A2) *_args[2] );
    }
    // void
    template<
            typename T,
            typename A0,
            typename A1,
            typename A2,
            typename F = T(A0, A1, A2),
            typename std::enable_if< std::is_void<T>::value, int>::type = 0>
    variant invoke(F *_function, const std::vector<variant *> &_args)
    {
        _function( (A0) *_args[0], (A1) *_args[1], (A2) *_args[2] );
        return null_t{};
    }

    /** 4 arg function */
    // non void
    template<
            typename T,
            typename A0,
            typename A1,
            typename A2,
            typename A3,
            typename F = T(A0, A1, A2, A3),
            typename std::enable_if< !std::is_void<T>::value, int>::type = 0>
    variant invoke(F *_function, const std::vector<variant *> &_args)
    {
        return _function( (A0) *_args[0], (A1) *_args[1], (A2) *_args[2], (A3) *_args[3] );
    }
    // void
    template<
            typename T,
            typename A0,
            typename A1,
            typename A2,
            typename A3,
            typename F = T(A0, A1, A2, A3),
            typename std::enable_if< std::is_void<T>::value, int>::type = 0>
    variant invoke(F *_function, const std::vector<variant *> &_args)
    {
        _function( (A0) *_args[0], (A1) *_args[1], (A2) *_args[2], (A3) *_args[3] );
        return null_t{};
    }

    /** 5 arg function */
    // non-void
    template<typename T,
            typename A0,
            typename A1,
            typename A2,
            typename A3,
            typename A4,
            typename F = T(A0, A1, A2, A3, A4),
            typename std::enable_if< !std::is_void<T>::value, int>::type = 0>
    variant invoke(F *_function, const std::vector<variant *> &_args)
    {
        return _function( (A0) *_args[0], (A1) *_args[1], (A2) *_args[2], (A3) *_args[3], (A4) *_args[4] );
    }
    // void
    template<typename T,
            typename A0,
            typename A1,
            typename A2,
            typename A3,
            typename A4,
            typename F = T(A0, A1, A2, A3, A4),
            typename std::enable_if< std::is_void<T>::value, int>::type = 0>
    variant invoke(F *_function, const std::vector<variant *> &_args)
    {
        _function( (A0) *_args[0], (A1) *_args[1], (A2) *_args[2], (A3) *_args[3], (A4) *_args[4] );
        return null_t{};
    }

    template<typename T>
    class invokable_static;

    /** Generic Invokable (works for static only) */
    template<typename T, typename... Args>
    class invokable_static<T(Args...)> : public iinvokable
    {
    public:
        using function_t  = T(Args...);
        using return_t    = T;
        using arguments_t = std::tuple<Args...>;

        invokable_static(function_t* _implem, const char* _name)
        : m_function_impl(_implem)
        , m_function_type(*func_type_builder<function_t>::with_id(_name))
        {
            NODABLE_ASSERT(m_function_impl)
        }

        ~invokable_static() override {}

        inline variant invoke(const std::vector<variant *> &_args = {}) const override
        {
            return Nodable::invoke<return_t, Args... >(m_function_impl, _args);
        }

        const func_type& get_type() const override { return m_function_type; }

    private:
        function_t* const m_function_impl;
        func_type         m_function_type;
    };

    class iinvokable_nonstatic
    {
    public:
        virtual ~iinvokable_nonstatic() {};
        virtual const func_type& get_type() const = 0;
        virtual variant invoke(void* _instance, const std::vector<variant *> &_args = {}) const = 0;
    };

    template<typename T>
    class invokable_nonstatic;

    /**
     * wrapper for NON STATIC methods ONLY
     */
    template<typename R, typename C, typename ...Ts>
    class invokable_nonstatic<R(C::*)(Ts...)> : public iinvokable_nonstatic // WIP...
    {
        using class_t    = C;
        using function_t = R(C::*)(Ts...);
        const func_type  m_method_type;
        function_t       m_method;
    public:

        invokable_nonstatic(R(C::* _method)(Ts...) , const char* _name)
        : m_method( _method )
        , m_method_type(*func_type_builder<function_t>::with_id(_name) )
        {
        }

        const func_type& get_type() const override { return m_method_type; };

        virtual variant invoke(void* _instance, const std::vector<variant *> &_args = {}) const override
        {
            return Nodable::invoke<R, C, Ts...>(reinterpret_cast<class_t *>(_instance), m_method, _args);
        };
    };

}