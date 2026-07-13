#pragma once
#include "Vec2.h"
#include "Axis.h"

namespace tools
{
    constexpr static Vec2 CENTER         = {0.f, 0.f};
    constexpr static Vec2 BOTTOM         = AXIS_Y;
    constexpr static Vec2 TOP            = -AXIS_Y;
    constexpr static Vec2 RIGHT          = AXIS_X;
    constexpr static Vec2 LEFT           = -AXIS_X;
    constexpr static Vec2 TOP_LEFT       = LEFT + TOP;
    constexpr static Vec2 TOP_RIGHT      = RIGHT + TOP;
    constexpr static Vec2 BOTTOM_LEFT    = LEFT + BOTTOM;
    constexpr static Vec2 BOTTOM_RIGHT   = RIGHT + BOTTOM;
}