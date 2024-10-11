#pragma once

#include <functional>
#include <tuple>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <cstddef>
#include "tools/core/types.h"
#include "variant.h"
#include "Type.h"
#include "FuncType.h"
#include "function_traits.h"

namespace tools
{

    class IInvokable
    {
    public:
        virtual ~IInvokable() = default;
        virtual const FuncType* get_sig() const = 0;
        virtual variant invoke(const std::vector<variant *> &_args) const = 0;
    };

    class IInvokableMethod
    {
    public:
        virtual ~IInvokableMethod() = default;
        virtual const FuncType* get_sig() const = 0;
        virtual variant invoke(void* _instance, const std::vector<variant *> &_args) const = 0;
    };

    template <typename ElementT, std::size_t... Indices>
    auto VectorToTupleEx(const std::vector<ElementT>& in_vector, std::index_sequence<Indices...>)
    {
        return std::make_tuple(in_vector[Indices]...);
    }

    template <std::size_t TUPLE_SIZE, typename ElementT>
    auto VectorToTuple(const std::vector<ElementT>& in_vector) // Convert a vector to a tuple,
    {
        VERIFY(in_vector.size() == TUPLE_SIZE, "Vector should have the expected size");
        return VectorToTupleEx( in_vector, std::make_index_sequence<TUPLE_SIZE>() );
    }

    // perform something close to std::apply but cast each argument to the FuncArgs[i] type.
    template<typename F, typename In>
    static auto CastAndApply(F* _function, In in)
    {
        using Args = typename FunctionTrait<F>::args_t;
        constexpr static size_t Args_SIZE = std::tuple_size_v<Args>;
        static_assert(  std::tuple_size_v<In> == Args_SIZE );
        // note: I could not figure out how to do std::apply(...) with a tuple while casting each of the elements to a given type.
        //       So I use N+1 manual std::invoke calls
        //
        // TODO: try to use switch/case to see if it works at compile-time
        //
        static_assert( std::tuple_size_v<In> == Args_SIZE );
        if constexpr ( Args_SIZE == 0 )
            return std::invoke( _function );
        if constexpr ( Args_SIZE == 1 )
            return std::invoke(
                _function,
                (std::tuple_element_t<0, Args>)*std::get<0>(in)
            );
        if constexpr ( Args_SIZE == 2 )
            return std::invoke(
                _function,
                (std::tuple_element_t<0, Args>)*std::get<0>(in),
                (std::tuple_element_t<1, Args>)*std::get<1>(in)
            );
        if constexpr ( Args_SIZE == 3 )
            return std::invoke(
                _function,
                (std::tuple_element_t<0, Args>)*std::get<0>(in),
                (std::tuple_element_t<1, Args>)*std::get<1>(in),
                (std::tuple_element_t<2, Args>)*std::get<2>(in)
            );
        if constexpr ( Args_SIZE == 4 )
            return std::invoke(
                _function,
                (std::tuple_element_t<0, Args>)*std::get<0>(in),
                (std::tuple_element_t<1, Args>)*std::get<1>(in),
                (std::tuple_element_t<2, Args>)*std::get<2>(in),
                (std::tuple_element_t<3, Args>)*std::get<3>(in)
            );
        if constexpr ( Args_SIZE == 5 )
            return std::invoke(
                _function,
                (std::tuple_element_t<0, Args>)*std::get<0>(in),
                (std::tuple_element_t<1, Args>)*std::get<1>(in),
                (std::tuple_element_t<2, Args>)*std::get<2>(in),
                (std::tuple_element_t<3, Args>)*std::get<3>(in),
                (std::tuple_element_t<4, Args>)*std::get<4>(in)
            );
    }

    // perform something close to std::apply but cast each argument to the FuncArgs[i] type.
    template<typename MethodT, typename InstanceT, typename ArgsT>
    static auto CastAndApply(MethodT _method, InstanceT _instance, ArgsT in)
    {
        using Args = typename FunctionTrait<MethodT>::args_t;
        constexpr static size_t Args_SIZE = std::tuple_size_v<Args>;
        static_assert(  std::tuple_size_v<ArgsT> == Args_SIZE );
        // note: I could not figure out how to do std::apply(...) with a tuple while casting each of the elements to a given type.
        //       So I use N+1 manual std::invoke calls
        //
        // TODO: try to use switch/case to see if it works at compile-time
        //
        static_assert( std::tuple_size_v<ArgsT> == Args_SIZE );
        if constexpr ( Args_SIZE == 0 )
            return std::invoke(_method, _instance);
        if constexpr ( Args_SIZE == 1 )
            return std::invoke(
                    _method, _instance,
                    (std::tuple_element_t<0, Args>)*std::get<0>(in)
            );
        if constexpr ( Args_SIZE == 2 )
            return std::invoke(
                    _method, _instance,
                    (std::tuple_element_t<0, Args>)*std::get<0>(in),
                    (std::tuple_element_t<1, Args>)*std::get<1>(in)
            );
        if constexpr ( Args_SIZE == 3 )
            return std::invoke(
                    _method, _instance,
                    (std::tuple_element_t<0, Args>)*std::get<0>(in),
                    (std::tuple_element_t<1, Args>)*std::get<1>(in),
                    (std::tuple_element_t<2, Args>)*std::get<2>(in)
            );
        if constexpr ( Args_SIZE == 4 )
            return std::invoke(
                    _method, _instance,
                    (std::tuple_element_t<0, Args>)*std::get<0>(in),
                    (std::tuple_element_t<1, Args>)*std::get<1>(in),
                    (std::tuple_element_t<2, Args>)*std::get<2>(in),
                    (std::tuple_element_t<3, Args>)*std::get<3>(in)
            );
        if constexpr ( Args_SIZE == 5 )
            return std::invoke(
                    _method, _instance,
                    (std::tuple_element_t<0, Args>)*std::get<0>(in),
                    (std::tuple_element_t<1, Args>)*std::get<1>(in),
                    (std::tuple_element_t<2, Args>)*std::get<2>(in),
                    (std::tuple_element_t<3, Args>)*std::get<3>(in),
                    (std::tuple_element_t<4, Args>)*std::get<4>(in)
            );
    }

    // apply a function with arguments
    template<typename F>
    variant Apply(F* _function, const std::vector<variant*> &_args)
    {
        using Args = typename FunctionTrait<F>::args_t;
        constexpr size_t N = std::tuple_size_v<Args>;
        VERIFY(_args.size() == N, "Wrong number of arguments");
        if constexpr ( std::is_void_v< typename std::result_of<F> > )
        {
            CastAndApply( _function, VectorToTuple<N>( _args ));
            return null_t{};
        } else {
            return CastAndApply( _function, VectorToTuple<N>( _args ));
        }
    };

    // apply a class member function to a given instance with arguments
    template<typename MethodPtrT, typename InstanceT = typename FunctionTrait<MethodPtrT>::ClassT >
    variant Apply(MethodPtrT _method, InstanceT _instance, const std::vector<variant*> &_args)
    {
        using Args = typename FunctionTrait<MethodPtrT>::args_t;
        constexpr size_t N = std::tuple_size_v<Args>;
        VERIFY(_args.size() == N, "Wrong number of arguments");
        if constexpr ( std::is_void_v< typename FunctionTrait<MethodPtrT>::result_t > )
        {
            CastAndApply(_method, _instance, VectorToTuple<N>( _args ));
            return null_t{};
        } else {
            return CastAndApply(_method, _instance, VectorToTuple<N>( _args ));
        }
    };

    /** Generic Invokable (works for static only) */
    template<typename FunctionT>
    class InvokableStaticFunction : public IInvokable
    {
    public:
        static_assert( std::is_function_v<FunctionT> );
        static_assert( !std::is_member_function_pointer_v<FunctionT> );

        InvokableStaticFunction(const FuncType* _function_type, const FunctionT* _function_pointer)
            : m_function_pointer( _function_pointer )
            , m_function_signature(_function_type)
        { ASSERT( m_function_pointer ) }

        variant invoke(const std::vector<variant *> &_args) const override
        { return tools::Apply( m_function_pointer, _args ); }

        const FuncType* get_sig() const override
        { return m_function_signature; }

    private:
        const FunctionT* m_function_pointer;
        const FuncType*  m_function_signature;
    };

    /**
     * wrapper for NON STATIC methods ONLY
     */
    template<typename MethodT>
    class InvokableMethod : public IInvokableMethod
    {
        using ClassT = typename FunctionTrait<MethodT>::class_t;
        static_assert( std::is_void_v<typename FunctionTrait<MethodT>::class_t> == false );
        static_assert( FunctionTrait<MethodT>::is_member_function );

        const FuncType* m_method_signature;
        MethodT         m_method_pointer;

    public:
        InvokableMethod(const FuncType* _method_type, MethodT _method_pointer )
            : m_method_pointer( _method_pointer )
            , m_method_signature(_method_type )
        { ASSERT( m_method_pointer ) }

        variant invoke( void* _instance, const std::vector<variant*>& _args ) const override
        {
            VERIFY(_instance != nullptr, "An instance is required!");
            return tools::Apply( m_method_pointer, (ClassT*)_instance, _args );
        };

        const FuncType* get_sig() const override
        { return m_method_signature; };
    };

}