#pragma once

namespace Nodable::R
{
    template<typename T>
    struct reflect_type;

    template<auto value>
    struct reflect_value;

    /** struct to link a type T0 with a value from a certain type T1*/
    template<typename T, auto value>
    struct type_to_value;
}