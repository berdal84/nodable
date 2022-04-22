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
        virtual void invoke(variant *_result, const std::vector<variant *> &_args = {}) const = 0;
    };

    /** Helpers to call a function (need serious work here) */

    /** 0 arg function */
    template<typename T, typename F = T()>
    void call(F *_function, variant *_result, const std::vector<variant *> &_args)
    {
        _result->set( _function() );
    }

    /** 1 arg function */
    template<typename T, typename A0, typename F = T(A0)>
    void call(F *_function, variant *_result, const std::vector<variant *> &_args)
    {
        _result->set( _function( (A0) *_args[0] ) );
    }

    /** 2 arg function */
    template<typename T, typename A0, typename A1, typename F = T(A0, A1)>
    void call(F *_function, variant *_result, const std::vector<variant *> &_args)
    {
        _result->set( _function( (A0) *_args[0], (A1) *_args[1] ) );
    }

    /** 3 arg function */
    template<typename T, typename A0, typename A1, typename A2, typename F = T(A0, A1, A2)>
    void call(F *_function, variant *_result, const std::vector<variant *> &_args)
    {
        _result->set( _function( (A0) *_args[0], (A1) *_args[1], (A2) *_args[2] ) );
    }

    /** 4 arg function */
    template<typename T, typename A0, typename A1, typename A2, typename A3, typename F = T(A0, A1, A2, A3)>
    void call(F *_function, variant *_result, const std::vector<variant *> &_args)
    {
        _result->set( _function( (A0) *_args[0], (A1) *_args[1], (A2) *_args[2], (A3) *_args[3] ) );
    }

    /** 5 arg function */
    template<typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename F = T(A0, A1, A2, A3, A4)>
    void call(F *_function, variant *_result, const std::vector<variant *> &_args)
    {
        _result->set( _function( (A0) *_args[0], (A1) *_args[1], (A2) *_args[2], (A3) *_args[3], (A4) *_args[4] ) );
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

        inline void invoke(variant *_result, const std::vector<variant *> &_args = {}) const override
        {
            call<return_t, Args... >(m_function_impl, _result, _args);
        }

        const func_type& get_type() const override { return m_function_type; }

    private:
        function_t* const m_function_impl;
        func_type         m_function_type;
    };

    /**
     * wrapper for NON STATIC methods ONLY
     */
    class invokable_nonstatic : public iinvokable // WIP...
    {
        const func_type m_method_type;
        void*           m_method;
    public:
        template<typename R, typename C, typename ...Ts>
        static void* func_ptr_cast(R(C::* _func_ptr)(Ts...))
        {
            union
            {
                R(C::* func_ptr)(Ts...);
                void*  ptr;
            } u;
            u.func_ptr = _func_ptr;
            return u.ptr;
        }

        template<typename F>
        static F func_ptr_cast(void* _ptr)
        {
            union
            {
                F func_ptr = nullptr; // is larger than void*
                void*  ptr;
            } u;
            u.ptr = _ptr;
            return u.func_ptr;
        }

        template<typename R, typename C, typename ...Ts>
        invokable_nonstatic(R(C::*_method)(Ts...) , const char* _name)
        : m_method( func_ptr_cast(_method) )
        , m_method_type(*func_type_builder<R(C::*)(Ts...)>::with_id(_name) )
        {
        }

        const func_type& get_type() const override { return m_method_type; };

        void invoke(variant *_result, const std::vector<variant *> &_args = {}) const override
        {
            NODABLE_ASSERT_EX(false, "Function not implemented!");
        };

        template<typename R, typename C, typename ...Ts>
        R invoke(C& _instance, Ts... args)
        {
            // TODO: cast to desired type is 100% unsafe! check types usint m_method_type
            auto method = func_ptr_cast<R(C::*)(Ts...)>(m_method);
            return (_instance.*method)(args...);
        }
    };

}