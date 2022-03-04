#pragma once

#include "R_internal_Typename.h"

#define CONCATENATE_DETAIL(x, y) x##y
#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)
#define MAKE_UNIQUE_VAR_NAME(x) CONCATENATE(x, __COUNTER__)

namespace Nodable::R
{
    /* Base interface for any meta class */
    class i_meta
    {
    public:
        virtual const char* get_name() const = 0;
        virtual Typename    get_typename() const = 0;
        virtual Qualifier   get_qualifier() const = 0;
        virtual bool        has_qualifier(Qualifier) const = 0;
        virtual void        add_qualifier(Qualifier) = 0;
    };

    // some meta_xxxx to get information at compile time
    template<typename T> class meta_type;
    template<Typename val> class meta_enum;
    template<typename T, Typename U> class meta_info;

    template<typename T>
    struct is_class_reflected
    {
        static constexpr bool value = std::is_member_function_pointer<decltype(&T::get_class)>::value;
    };
}