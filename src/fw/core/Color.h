#pragma once
#include "Vec4.h"
#include "types.h"

namespace fw
{
    class Color
    {
    public:
        Vec4 value;

        Color( u8_t r, u8_t g, u8_t b, u8_t a = 255 )
        : value(
            float( r ) / 255.f,
            float( g ) / 255.f,
            float( b ) / 255.f,
            float( a ) / 255.f
          )
        {}

        operator Vec4() const
        { return value; }
    };
}