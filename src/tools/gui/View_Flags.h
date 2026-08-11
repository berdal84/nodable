#pragma once
#include <tools/core/Types.h>

namespace tools
{
    typedef u16_t View_Flags;
    enum View_Flag_
    {
        View_Flag_NONE          = 0,
        View_Flag_PINNED        = 1 << 0,
        View_Flag_SELECTED      = 1 << 1,
        View_Flag_VISIBLE       = 1 << 2, 
        View_Flag_HOVERED       = 1 << 3,

        View_Flag_DEFAULTS      = View_Flag_VISIBLE // TODO: mirror View_Flag_SELECTED to View_Flag_HIDDEN,  we want 0 to be default
        // MAX!        = 1 << 7,
    };
}
