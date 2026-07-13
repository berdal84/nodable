#pragma once
#include "Vec2.h"

namespace tools
{
    constexpr static Vec2 AXIS_X  = {1.f, 0.f};
    constexpr static Vec2 AXIS_Y  = {0.f, 1.f};
    constexpr static Vec2 AXIS_XY = AXIS_X + AXIS_Y;
}