#pragma once

#include "Operator.h"
#include "Initializer.h"
#include "Enum.h"
#include "Type_Descriptor.h"
#include "Type_Register.h"
#include "Union.h"

namespace ndbl
{
    static void reflection_init()
    {
        DEFINE_REFLECT(double);
        DEFINE_REFLECT(bool);
        DEFINE_REFLECT(void);
        DEFINE_REFLECT(void*);
        DEFINE_REFLECT(any);
        DEFINE_REFLECT(null);

        DEFINE_REFLECT_WITH_ALIAS(bdc::String, "string");
        DEFINE_REFLECT_WITH_ALIAS(i8_t       , "i8");
        DEFINE_REFLECT_WITH_ALIAS(i16_t      , "i16");
        DEFINE_REFLECT_WITH_ALIAS(i32_t      , "i32");
        DEFINE_REFLECT_WITH_ALIAS(i64_t      , "i64");
    }

    static void reflection_shutdown()
    {
        type_register_clear();
    }
}