#pragma once

namespace Nodable
{
    namespace R
    {
        template<typename T>
        struct reflect_type;

        template<Type value>
        struct reflect_value;

        template<typename T, Type value>
        struct type_to_value;
    }
}