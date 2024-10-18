#pragma once

namespace tools
{
    // to distinguish the referential of a position
    typedef int Space;
    enum Space_: int
    {
        UNKNOWN_SPACE  = 0,
        LOCAL_SPACE    = 1, // Relative to object's position
        PARENT_SPACE   = 2, // Relative to parent's position (usually it is the default)
        WORLD_SPACE    = 3, // Relative to the world (in 2d: screen's top-left corner)
        // WINDOW_SPACE = 4 // Not implemented yet!
    };

}