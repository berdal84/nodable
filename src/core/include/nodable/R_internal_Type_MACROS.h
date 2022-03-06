#pragma once

#define R_DEF_TYPENAME( CPP_T, REFLECT_T ) \
    template<> \
    class meta_info<CPP_T, REFLECT_T> : public Type \
    {  \
    public:\
        /* compile time */ \
        using                        cpp_t         = CPP_T; \
        /* runtime (static) */                      \
        static constexpr const char* cpp_name      = #CPP_T; \
        static constexpr Typename    reflect_t     = REFLECT_T; \
        static constexpr const char* reflect_name  = #REFLECT_T;\
        /* runtime (dynamic) */ \
        meta_info(): Type(#CPP_T, #REFLECT_T, REFLECT_T) {} \
    };\
    template<>\
    class meta_enum<REFLECT_T> : public meta_info<CPP_T, REFLECT_T> {}; \
    \
    template<>\
    class meta_type<CPP_T> : public meta_info<CPP_T, REFLECT_T> {};\
    \
    static register_type<CPP_T, REFLECT_T> MAKE_UNIQUE_VAR_NAME(inserter);
