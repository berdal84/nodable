#pragma once

#include <functional>
#include <tuple>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */

#include <nodable/Nodable.h>
#include <nodable/Reflect.h>
#include <nodable/Member.h>
#include <nodable/Invokable.h>
#include <nodable/FunctionSignature.h>

namespace Nodable {

    /** Helpers to call a function (need serious work here) */

    /** 0 arg function */
    template<typename R, typename F = R()>
    void call(F *_function, Member *_result, const std::vector<Member *> &_args)
    {
        _result->set( _function() );
    }

    /** 1 arg function */
    template<typename R, typename A0, typename F = R(A0)>
    void call(F *_function, Member *_result, const std::vector<Member *> &_args)
    {
        _result->set( _function( (A0) *_args[0] ) );
    }

    /** 2 arg function */
    template<typename R, typename A0, typename A1, typename F = R(A0, A1)>
    void call(F *_function, Member *_result, const std::vector<Member *> &_args)
    {
        _result->set( _function( (A0) *_args[0], (A1) *_args[1] ) );
    }

    /** 3 arg function */
    template<typename R, typename A0, typename A1, typename A2, typename F = R(A0, A1, A2)>
    void call(F *_function, Member *_result, const std::vector<Member *> &_args)
    {
        _result->set( _function( (A0) *_args[0], (A1) *_args[1], (A2) *_args[2] ) );
    }

    /** 4 arg function */
    template<typename R, typename A0, typename A1, typename A2, typename A3, typename F = R(A0, A1, A2, A3)>
    void call(F *_function, Member *_result, const std::vector<Member *> &_args)
    {
        _result->set( _function( (A0) *_args[0], (A1) *_args[1], (A2) *_args[2], (A3) *_args[3] ) );
    }

    /** 5 arg function */
    template<typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename F = R(A0, A1, A2, A3, A4)>
    void call(F *_function, Member *_result, const std::vector<Member *> &_args)
    {
        _result->set( _function( (A0) *_args[0], (A1) *_args[1], (A2) *_args[2], (A3) *_args[3], (A4) *_args[4] ) );
    }


    template<typename T>
    class InvokableFunction;

    /** Generic Invokable Function */
    template<typename R, typename... Args>
    class InvokableFunction<R(Args...)> : public Invokable
    {
    public:
        using   FunctionType = R(Args...);
        using   ArgTypes     = std::tuple<Args...>;

        InvokableFunction(FunctionType* _function, const char* _identifier, const char* _label = "")
        {
            m_function  = _function;
            m_signature = FunctionSignature::new_instance<FunctionType>::with_id(_identifier, _label );
        }

        ~InvokableFunction()
        {
            delete m_function;
            delete m_signature;
        }

        inline void invoke(Member *_result, const std::vector<Member *> &_args) const override
        {
            call<R, Args...>(m_function, _result, _args);
        }

        inline const FunctionSignature* get_signature() const override { return m_signature; };
        inline Invokable::Type          get_invokable_type() const override { return Invokable::Type::Function; };
    private:
        FunctionType*      m_function;
        FunctionSignature* m_signature;
    };

}