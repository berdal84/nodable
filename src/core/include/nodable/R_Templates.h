#pragma once

namespace Nodable::R
{
    template<typename T>
    struct reflect_type;

    template<typename T, T value>
    struct reflect_value;

    /** struct to link a type T0 with a value from a certain type T1*/
    template<typename T0, typename T1, T1 value>
    struct type_to_value;
}