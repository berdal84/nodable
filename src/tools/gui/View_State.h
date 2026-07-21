#pragma once
#include <tools/core/Types.h>

namespace tools
{
    typedef u8_t View_Flags;
    enum View_Flag_
    {
        View_Flag_NONE          = 0,
        View_Flag_PINNED        = 1 << 0,
        View_Flag_SELECTED      = 1 << 1,
        View_Flag_VISIBLE       = 1 << 2, // TODO: mirror this to View_Flag_HIDDEN
        View_Flag_HOVERED       = 1 << 3,

        View_Flag_DEFAULTS      = View_Flag_VISIBLE
        // MAX!        = 1 << 7,
    };

    /**
     * Simple struct to hold some flags and getters/setters to modify them easily.
     * See examples in Node_View or Node_Slot_View
     */
	struct View_State
	{
        View_Flags flags = View_Flag_DEFAULTS;

        bool    has_flags(View_Flags _flag) const           { return (flags & _flag) == _flag; }
        void    set_flags(View_Flags _flag, bool on = true) { flags = (flags & ~_flag) | ( on * _flag   ) ;}
    };
}
