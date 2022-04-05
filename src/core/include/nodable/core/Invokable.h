#pragma once

#include <functional>
#include <tuple>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */

#include <nodable/core/types.h>
#include <nodable/core/reflection/R.h>
#include <nodable/core/Member.h>
#include <nodable/core/IInvokable.h>
#include <nodable/core/Signature.h>

namespace Nodable {

    /** Helpers to call a function (need serious work here) */

    /** 0 arg function */
    template<typename T, typename F = T()>
    void call(F *_function, Member *_result, const std::vector<Member *> &_args)
    {
        _result->set( _function() );
    }

    /** 1 arg function */
    template<typename T, typename A0, typename F = T(A0)>
    void call(F *_function, Member *_result, const std::vector<Member *> &_args)
    {
        _result->set( _function( (A0) *_args[0] ) );
    }

    /** 2 arg function */
    template<typename T, typename A0, typename A1, typename F = T(A0, A1)>
    void call(F *_function, Member *_result, const std::vector<Member *> &_args)
    {
        _result->set( _function( (A0) *_args[0], (A1) *_args[1] ) );
    }

    /** 3 arg function */
    template<typename T, typename A0, typename A1, typename A2, typename F = T(A0, A1, A2)>
    void call(F *_function, Member *_result, const std::vector<Member *> &_args)
    {
        _result->set( _function( (A0) *_args[0], (A1) *_args[1], (A2) *_args[2] ) );
    }

    /** 4 arg function */
    template<typename T, typename A0, typename A1, typename A2, typename A3, typename F = T(A0, A1, A2, A3)>
    void call(F *_function, Member *_result, const std::vector<Member *> &_args)
    {
        _result->set( _function( (A0) *_args[0], (A1) *_args[1], (A2) *_args[2], (A3) *_args[3] ) );
    }

    /** 5 arg function */
    template<typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename F = T(A0, A1, A2, A3, A4)>
    void call(F *_function, Member *_result, const std::vector<Member *> &_args)
    {
        _result->set( _function( (A0) *_args[0], (A1) *_args[1], (A2) *_args[2], (A3) *_args[3], (A4) *_args[4] ) );
    }

    template<typename T>
    class Invokable;

    /** Generic Invokable Function */
    template<typename T, typename... Args>
    class Invokable<T(Args...)> : public IInvokable
    {
        using F = T(Args...);
        Invokable(F* _function, const Signature* _sig)
        : m_function(_function)
        , m_signature(_sig)
        {
            NODABLE_ASSERT(_function)
            NODABLE_ASSERT(_sig)
        }
    public:

        static IInvokable* new_function(F* _function, const char* _id)
        {
            NODABLE_ASSERT(_function)
            Signature* sig = Signature::from_type<F>::as_function(_id);
            return new Invokable(_function, sig);
        }

        static IInvokable* new_operator(F* _function, const Operator* _op)
        {
            NODABLE_ASSERT(_function)
            NODABLE_ASSERT(_op)
            Signature* sig = Signature::from_type<F>::as_operator(_op);
            return new Invokable(_function, sig);
        }

        ~Invokable() override
        {
            // delete m_function; no need to
            delete m_signature;
        }

        inline void invoke(Member *_result, const std::vector<Member *> &_args) const override
        {
            call<T, Args...>(m_function, _result, _args);
            for(auto arg : _args)
            {
                if ( arg->is_connected_by(ConnectBy_Ref) )
                {
                    arg->force_defined_flag(true);
                }
            }
        }
        const Signature*   get_signature() const override { return m_signature; };

    private:
        F*               m_function;
        const Signature* m_signature;
    };

}