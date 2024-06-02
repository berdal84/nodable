#pragma once
#include "geometry/Vec4.h"
#include "types.h"

namespace tools
{
    class Color
    {
    public:
        Vec4 value;

        constexpr Color() = default;
        constexpr Color( u8_t r, u8_t g, u8_t b, u8_t a = 255 )
        : value(
            float( r ) / 255.f,
            float( g ) / 255.f,
            float( b ) / 255.f,
            float( a ) / 255.f
          )
        {}

        constexpr operator Vec4() const
        { return value; }
    };
}