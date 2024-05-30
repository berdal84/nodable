#pragma once

#include <functional>
#include <tuple>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <cstddef>
#include "tools/core/types.h"
#include "variant.h"
#include "type.h"
#include "func_type.h"
#include "function_traits.h"

namespace tools
{

    class IInvokable
    {
    public:
        virtual ~IInvokable() = default;
        virtual const func_type* get_type() const = 0;
        virtual variant invoke(const std::vector<variant *> &_args) const = 0;
    };

    class IInvokableMethod
    {
    public:
        virtual ~IInvokableMethod() = default;
        virtual const func_type* get_type() const = 0;
        virtual void bind(void* _instance) = 0; // Bind a given instance to the invokable
        virtual void unbind() = 0;
        virtual variant invoke(const std::vector<variant *> &_args) const = 0; // call the invokable on the currently bound instance, with the given arguments.
    };

    template <typename ElementT, std::size_t... Indices>
    auto VectorToTupleEx(const std::vector<ElementT>& in_vector, std::index_sequence<Indices...>)
    {
        return std::make_tuple(in_vector[Indices]...);
    }

    template <std::size_t TUPLE_SIZE, typename ElementT>
    auto VectorToTuple(const std::vector<ElementT>& in_vector) // Convert a vector to a tuple,
    {
        EXPECT(in_vector.size() == TUPLE_SIZE, "Vector should have the expected size");
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
        EXPECT(_args.size() == N, "Wrong number of arguments");
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
        EXPECT(_args.size() == N, "Wrong number of arguments");
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

        InvokableStaticFunction(FunctionT* _function_pointer, const char* _name)
            : m_function_pointer( _function_pointer )
            , m_function_type(func_type_builder<FunctionT>::with_id(_name))
        { ASSERT( m_function_pointer ) }

        ~InvokableStaticFunction() override
        { delete m_function_type; }

        variant invoke(const std::vector<variant *> &_args) const override
        { return tools::Apply( m_function_pointer, _args ); }

        const func_type* get_type() const override
        { return m_function_type; }

    private:
        FunctionT* const m_function_pointer;
        func_type*       m_function_type;
    };

    /**
     * wrapper for NON STATIC methods ONLY
     */
    template<typename MethodT>
    class InvokableMethod : public IInvokableMethod// WIP...
    {
        using ClassT = typename FunctionTrait<MethodT>::class_t;
        static_assert( std::is_void_v<typename FunctionTrait<MethodT>::class_t> == false );
        static_assert( FunctionTrait<MethodT>::is_member_function );

        const func_type* m_method_type;
        MethodT          m_method_pointer;
        ClassT*          m_bound_instance{ nullptr };

    public:
        InvokableMethod( MethodT _method_pointer, const char* _name )
            : m_method_pointer( _method_pointer )
            , m_method_type( func_type_builder<MethodT>::with_id( _name ) )
        { ASSERT( m_method_pointer ) }

        ~InvokableMethod() override
        { delete m_method_type; }

        void bind(void* _instance) override
        { ASSERT(m_bound_instance != nullptr); m_bound_instance = reinterpret_cast<ClassT*>( _instance );  }

        void unbind() override
        { m_bound_instance = nullptr; }

        variant invoke( const std::vector<variant*>& _args ) const override
        {
            EXPECT(m_bound_instance != nullptr, "No instance bound. Call bind() prior to call invoke()");
            return tools::Apply( m_method_pointer, m_bound_instance, _args );
        };

        const func_type* get_type() const override
        { return m_method_type; };
    };

}