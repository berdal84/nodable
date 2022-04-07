#pragma once

namespace Nodable
{
    namespace R
    {
        template<typename T>
        struct reflect_t;

        template<Type value>
        struct reflect_v;

        template<typename T, Type value>
        struct type_to_value;
    }
}