#pragma once

/**
 * These 3 macros allows to call to_string(E enum) with a given enum E as argument.
 *
 * enum class MyEnum
 * {
 *      MyEnum_A,
 *      MyEnum_B,
 *      ...
 * }
 *
 * REFLECT_ENUM_CLASS(MyEnum)
 * REFLECT_ENUM_CLASS_VALUE(A) // Works for regular enums too using REFLECT_ENUM_VALUE.
 * REFLECT_ENUM_CLASS_VALUE(B)
 * ...
 * REFLECT_ENUM_CLASS_END()
 *
 * const char* str = to_string(MyEnum::A); // str == "A"
 */


#define REFLECT_ENUM( ENUM ) \
    static const char* ENUM##_to_string(ENUM value) \
    {\
        switch( value )\
        {                    \
        __REFLECT_ENUM_CONTENT

#define REFLECT_ENUM_V(VALUE) \
            case VALUE: return #VALUE;

#define __REFLECT_ENUM_CONTENT(...) \
            __VA_ARGS__ \
            default: return "<not reflected>";\
        } \
    }

#define REFLECT_ENUM_CLASS( ENUM ) \
    static const char* to_string(ENUM value) \
    {\
        using EnumT = ENUM;\
        switch( value )\
        {\
    __REFLECT_ENUM_CONTENT

#define REFLECT_ENUM_CLASS_V(VALUE) \
            case EnumT::VALUE: return #VALUE;
