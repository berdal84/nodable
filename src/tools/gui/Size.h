#pragma once

#include "tools/gui/geometry/Vec2.h"
#include "tools/core/assertions.h"

namespace tools
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