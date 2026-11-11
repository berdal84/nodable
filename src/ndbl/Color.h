#pragma once
#include "bdc/Types.hpp"
#include "geometry/Vec4.h"

namespace ndbl
{
    // Color utilities
    //
    // Note: A color is a Vec4 (float precision)

    constexpr Vec4 color_from_rgba_u8( u8_t r, u8_t g, u8_t b, u8_t a = 255 )
    {
        return Vec4{
            float( r ) / 255.f,
            float( g ) / 255.f,
            float( b ) / 255.f,
            float( a ) / 255.f   
        };
    };
}