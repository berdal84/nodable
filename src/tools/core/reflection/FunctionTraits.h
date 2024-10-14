#pragma once
#include <type_traits>

namespace tools
{
    template<typename ResultT, typename Args, typename ClassT = void>
    struct FunctionTraitEx
    {
        using args_t   = Args;
        using result_t = ResultT;
        using class_t  = ClassT;
        static constexpr bool is_member_function = std::is_void_v<ClassT> == false;

        static_assert( std::is_class_v<ClassT> || std::is_void_v<ClassT> );
    };

    template<typename R, typename ...Args>
    struct FunctionTrait;

    template<typename ResultT, typename ...Args>
    struct FunctionTrait<ResultT(Args...)> // regular function
        : FunctionTraitEx<ResultT, std::tuple<Args...>>
    {
        static_assert( std::is_member_function_pointer_v<ResultT(Args...)> == false );
    };

    template<typename ResultT, typename ClassT,  typename ...Args>
    struct FunctionTrait<ResultT(ClassT::*)(Args...)> // member function
        : FunctionTraitEx<ResultT, std::tuple<Args...>, ClassT>
    {};
}