#pragma once

#include "R_MACROS.h"

namespace Nodable
{
    namespace R
    {
        /**
         * To distinguish base types.
         */
        enum class Type : unsigned short int {
            null_t = 0,
            void_t,
            bool_t,
            double_t,
            i16_t,
            string_t,
            Class,
            COUNT
        };

        // reflect the enum
        R_ENUM(Type)
        R_ENUM_VALUE(null_t)
        R_ENUM_VALUE(void_t)
        R_ENUM_VALUE(bool_t)
        R_ENUM_VALUE(i16_t)
        R_ENUM_VALUE(double_t)
        R_ENUM_VALUE(string_t)
        R_ENUM_VALUE(Class)
        R_ENUM_VALUE(COUNT)
        R_ENUM_END
    }
}