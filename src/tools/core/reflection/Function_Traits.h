#pragma once
#include <tuple>

namespace tools
{
    template<typename _Result_Type, typename _Args_Type, typename _Class_Type = void>
    struct Function_Trait_Ex
    {
        using Args_Type   = _Args_Type;
        using Result_Type = _Result_Type;
        using Class_Type  = _Class_Type;
        static constexpr bool is_member_function = std::is_void_v<Class_Type> == false;

        static_assert( std::is_class_v<Class_Type> || std::is_void_v<Class_Type> );
    };

    template<typename Result_Type, typename ...Args_Type>
    struct Function_Trait;

    template<typename Result_Type, typename ...Args_Type>
    struct Function_Trait<Result_Type(Args_Type...)> // regular function
        : Function_Trait_Ex<Result_Type, std::tuple<Args_Type...>>
    {
        static_assert( std::is_member_function_pointer_v<Result_Type(Args_Type...)> == false );
    };

    template<typename Result_Type, typename Class_Type,  typename ...Args_Type>
    struct Function_Trait<Result_Type(Class_Type::*)(Args_Type...)> // member function
        : Function_Trait_Ex<Result_Type, std::tuple<Args_Type...>, Class_Type>
    {};
}