#pragma once
#include "Vec2.h"

namespace tools
{
    constexpr static Vec2 CENTER         = {.5f, 0.5f};
    constexpr static Vec2 BOTTOM         = {.5f, 1.0f};
    constexpr static Vec2 TOP            = {.5f, 0.0f};
    constexpr static Vec2 RIGHT          = {1.f, 0.5f};
    constexpr static Vec2 LEFT           = {0.f, 0.5f};
    constexpr static Vec2 TOP_LEFT       = {0.f, 0.f};
    constexpr static Vec2 TOP_RIGHT      = {1.f, 0.f };
    constexpr static Vec2 BOTTOM_LEFT    = {0.f, 1.0f};
    constexpr static Vec2 BOTTOM_RIGHT   = {1.f, 1.f};
}