#pragma once
#include <tools/core/Types.h>

namespace tools
{
    typedef u16_t View_Flags;
    enum View_Flag_
    {
        View_Flag_NULL          = 0,
        View_Flag_PINNED        = 1 << 0,
        View_Flag_SELECTED      = 1 << 1,
        View_Flag_HIDDEN        = 1 << 2, 
        View_Flag_HOVERED       = 1 << 3,
    };
}
