#pragma once

#include "ndbl/gui/geometry/Vec2.h"
#include "ndbl/core/Asserts.h"

namespace ndbl
{
    enum Size : int
    {
        Size_SM = 0, // Small
        Size_MD, // Medium
        Size_LG, // Large
        Size_XL, // Extra-Large
        Size_COUNT,
        Size_DEFAULT = Size_MD,
    };
}