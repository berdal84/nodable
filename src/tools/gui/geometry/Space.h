#pragma once

namespace tools
{
    // to distinguish the referential of a position
    typedef int Space;
    enum Space_: int
    {
        DEFAULT_SPACE = 0,
        SCREEN_SPACE  = DEFAULT_SPACE,
        LOCAL_SPACE   = 1,
        PARENT_SPACE  = 2,
    };

}