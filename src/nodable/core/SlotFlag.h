#pragma once
#include "fw/core/types.h"

namespace ndbl
{
    // Nature of the connection allowed by a given Slot
    typedef int SlotFlags;
    enum SlotFlag
    {
        // We see TYPEs as
        // DEPENDENT --- TYPE ---> DEPENDENCY

        SlotFlag_NONE              = 0,

        SlotFlag_ACCEPTS_DEPENDENTS   = 1 << 0,
        SlotFlag_ACCEPTS_DEPENDENCIES = 1 << 1,

        SlotFlag_TYPE_VALUE        = 1 << 2,
        SlotFlag_TYPE_HIERARCHICAL = 1 << 3,
        SlotFlag_TYPE_CODEFLOW     = 1 << 4,

        SlotFlag_INPUT             = SlotFlag_TYPE_VALUE        | SlotFlag_ACCEPTS_DEPENDENCIES,
        SlotFlag_OUTPUT            = SlotFlag_TYPE_VALUE        | SlotFlag_ACCEPTS_DEPENDENTS,
        SlotFlag_PARENT            = SlotFlag_TYPE_HIERARCHICAL | SlotFlag_ACCEPTS_DEPENDENCIES,
        SlotFlag_CHILD             = SlotFlag_TYPE_HIERARCHICAL | SlotFlag_ACCEPTS_DEPENDENTS,
        SlotFlag_PREV              = SlotFlag_TYPE_CODEFLOW     | SlotFlag_ACCEPTS_DEPENDENCIES,
        SlotFlag_NEXT              = SlotFlag_TYPE_CODEFLOW     | SlotFlag_ACCEPTS_DEPENDENTS,

        SlotFlag_ACCEPTS_MASK      = SlotFlag_ACCEPTS_DEPENDENTS | SlotFlag_ACCEPTS_DEPENDENCIES,
        SlotFlag_TYPE_MASK         = SlotFlag_TYPE_CODEFLOW | SlotFlag_TYPE_HIERARCHICAL | SlotFlag_TYPE_VALUE,
    };
}