#pragma once

#include <functional>
#include <tuple>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <cstddef>
#include "core/Asserts.h"
#include "Variant.h"
#include "Type_Descriptor.h"
#include "Function_Traits.h"

namespace tools
{

    class IInvokable
    {
    public:
        virtual ~IInvokable() = default;
        virtual const Function_Descriptor* get_sig() const = 0;
        virtual Variant invoke(const std::vector<Variant *> &_args) const = 0;
    };

    class IInvokable_Method
    {
    public:
        virtual ~IInvokable_Method() = default;
        virtual const Function_Descriptor* get_sig() const = 0;
        virtual Variant invoke(void* _instance, const std::vector<Variant *> &_args) const = 0;
    };

    template <typename ElementT, std::size_t... Indices>
    auto vector_to_tuple_ex(const std::vector<ElementT>& in_vector, std::index_sequence<Indices...>)
    {
        return std::make_tuple(in_vector[Indices]...);
    }

    template <std::size_t TUPLE_SIZE, typename ElementT>
    auto vector_to_tuple(const std::vector<ElementT>& in_vector) // Convert a vector to a tuple,
    {
        VERIFY(in_vector.size() == TUPLE_SIZE, "Vector should have the expected set_size");
        return vector_to_tuple_ex( in_vector, std::make_index_sequence<TUPLE_SIZE>() );
    }

    // perform something close to std::apply but cast each argument to the FuncArgs[i] type.
    template<typename F, typename In>
    static auto cast_and_apply(F* _function, In in)
    {
        using Args = typename Function_Trait<F>::Args_Type;
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
    static auto cast_and_apply(MethodT _method, InstanceT _instance, ArgsT in)
    {
        using Args = typename Function_Trait<MethodT>::Args_Type;
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
    Variant apply(F* _function, const std::vector<Variant*> &_args)
    {
        using trait = Function_Trait<F>;
        constexpr size_t N = std::tuple_size_v<typename trait::Args_Type>;
        VERIFY(_args.size() == N, "Wrong number of arguments");
        if constexpr ( std::is_void_v< typename trait::result_t > )
        {
            cast_and_apply( _function, vector_to_tuple<N>( _args ));
            return null{};
        } else {
            return cast_and_apply( _function, vector_to_tuple<N>( _args ));
        }
    };

    // apply a class member function to a given instance with arguments
    template<typename MethodPtrT, typename InstanceT = typename Function_Trait<MethodPtrT>::ClassT >
    Variant apply(MethodPtrT _method, InstanceT _instance, const std::vector<Variant*> &_args)
    {
        using trait = Function_Trait<MethodPtrT>;
        constexpr size_t N = std::tuple_size_v<typename trait::Args_Type>;
        VERIFY(_args.size() == N, "Wrong number of arguments");
        if constexpr ( std::is_void_v< typename trait::result_t >  )
        {
            cast_and_apply(_method, _instance, vector_to_tuple<N>( _args ));
            return null{};
        } else {
            return cast_and_apply(_method, _instance, vector_to_tuple<N>( _args ));
        }
    };

    /** Generic Invokable (works for static only) */
    template<typename FunctionT>
    class Invokable_Static_Function : public IInvokable
    {
    public:
        static_assert( std::is_function_v<FunctionT> );
        static_assert( !std::is_member_function_pointer_v<FunctionT> );

        Invokable_Static_Function(const bdc::String _name, const FunctionT* _function_pointer)
        : m_function_pointer( _function_pointer )
        {
            ASSERT( m_function_pointer );
            m_function_signature.init<FunctionT>(_name);
        }

        Variant invoke(const std::vector<Variant *> &_args) const override
        { return tools::apply( m_function_pointer, _args ); }

        const Function_Descriptor* get_sig() const override
        { return &m_function_signature; }

    private:
        const FunctionT*    m_function_pointer;
        Function_Descriptor m_function_signature;
    };

    /**
     * wrapper for NON STATIC methods ONLY
     */
    template<typename Method_Type>
    class Invokable_Method : public IInvokable_Method
    {
        using ClassT = typename Function_Trait<Method_Type>::Class_Type;
        static_assert( std::is_void_v<typename Function_Trait<Method_Type>::Class_Type> == false );
        static_assert( Function_Trait<Method_Type>::is_member_function );

        const Function_Descriptor* m_method_signature;
        Method_Type                m_method_pointer;

    public:
        Invokable_Method(const bdc::String _name, Method_Type _method_pointer )
            : m_method_pointer( _method_pointer )
            , m_method_signature(Function_Descriptor::create<Method_Type>(_name) )
        { ASSERT( m_method_pointer ); }

        Variant invoke( void* _instance, const std::vector<Variant*>& _args ) const override
        {
            VERIFY(_instance != nullptr, "An instance is required!");
            return tools::apply( m_method_pointer, (ClassT*)_instance, _args );
        };

        const Function_Descriptor* get_sig() const override
        { return m_method_signature; };
    };

}