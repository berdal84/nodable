#pragma once

/**
 * These 3 macros allows to call to_string(E enum) with a given enum E as argument.
 *
 * enum class MyEnum {
 *      MyEnum_A,
 *      MyEnum_B,
 *      ...
 * }
 *
 * R_ENUM(MyEnum)
 * R_ENUM_VALUE(A)
 * R_ENUM_VALUE(B)
 * ...
 * R_ENUM_END()
 *
 * const char* str = to_string(MyEnum::A); // str == "A"
 */

#define R_ENUM( Enum ) \
    static const char* to_string(Enum value) \
    {\
        using type = Enum; \
        switch( value )\
        {

#define R_ENUM_VALUE(VALUE) \
            case type::VALUE: return #VALUE;

#define R_ENUM_END \
            default: return "<not reflected>";\
        } \
    }
