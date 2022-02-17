#pragma once

#define R_DEF_META( CPP_T, REFLECT_T ) \
    template<> \
    struct meta_info<CPP_T, REFLECT_T> \
    {  \
        using                        cpp_t         = CPP_T; \
        static constexpr const char* cpp_name      = #CPP_T; \
        static constexpr Type        reflect_t     = REFLECT_T; \
        static constexpr const char* reflect_name  = #REFLECT_T; \
    };

#define R_TO_CPP( CPP_T, REFLECT_T ) \
    template<> \
    struct type<REFLECT_T> \
    {  \
        using meta = meta_info<CPP_T, REFLECT_T>; \
    };

#define CPP_TO_R( CPP_T, REFLECT_T ) \
    template<> \
    struct cpp<CPP_T> \
    {  \
        using meta = meta_info<CPP_T, REFLECT_T>; \
    };

#define R_DEF_TYPE( CPP_T, REFLECT_T ) \
    R_DEF_META( CPP_T, REFLECT_T ) \
    CPP_TO_R( CPP_T, REFLECT_T ) \
    R_TO_CPP( CPP_T, REFLECT_T )
