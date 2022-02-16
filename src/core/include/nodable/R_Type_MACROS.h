#pragma once

#define __R_TO_CPP( cpp_T, reflect_T, cpp_name, reflect_name ) \
    template<> \
    struct Nodable::R::type<reflect_T> \
    {  \
        static constexpr const char* name         = reflect_name; \
        using                        cpp_t        = cpp_T; \
        using                        r_cpp_t      = cpp_t; \
        static constexpr const char* r_cpp_t_name = cpp_name; \
    };

#define __CPP_TO_R( cpp_T, reflect_T, cpp_name, reflect_name ) \
    template<> \
    struct Nodable::R::cpp<cpp_T> \
    {  \
        using                        cpp_t           = cpp_T; \
        using                        r_cpp_t         = cpp_t; \
        static constexpr const char* name            = cpp_name; \
        static constexpr Type        reflect_t       = reflect_T; \
        static constexpr const char* reflect_t_name  = reflect_name; \
    }; \

#define R_DEF_TYPE( cpp_T, reflect_T ) \
    __CPP_TO_R( cpp_T, reflect_T, #cpp_T, #reflect_T ) \
    __R_TO_CPP( cpp_T, reflect_T, #cpp_T, #reflect_T )
