#pragma once
#include "fw/core/types.h"

namespace ndbl
{
    // Nature of the connection allowed by a given Slot
    typedef i8_t SlotFlags;
    enum SlotFlag : i8_t // require less bits, but reserve some.
    {
        SlotFlag_NONE              = 0,

        SlotFlag_ORDER_PRIMARY     = 1 << 0,
        SlotFlag_ORDER_SECONDARY   = 1 << 1,

        SlotFlag_TYPE_VALUE        = 1 << 2,
        SlotFlag_TYPE_HIERARCHICAL = 1 << 3,
        SlotFlag_TYPE_CODEFLOW     = 1 << 4,

        SlotFlag_OUTPUT            = SlotFlag_TYPE_VALUE        | SlotFlag_ORDER_PRIMARY,
        SlotFlag_INPUT             = SlotFlag_TYPE_VALUE        | SlotFlag_ORDER_SECONDARY,
        SlotFlag_PARENT            = SlotFlag_TYPE_HIERARCHICAL | SlotFlag_ORDER_PRIMARY,
        SlotFlag_CHILD             = SlotFlag_TYPE_HIERARCHICAL | SlotFlag_ORDER_SECONDARY,
        SlotFlag_PREV              = SlotFlag_TYPE_CODEFLOW     | SlotFlag_ORDER_PRIMARY,
        SlotFlag_NEXT              = SlotFlag_TYPE_CODEFLOW     | SlotFlag_ORDER_SECONDARY,

        SlotFlag_ORDER_MASK        = SlotFlag_ORDER_PRIMARY | SlotFlag_ORDER_SECONDARY,
        SlotFlag_TYPE_MASK         = SlotFlag_TYPE_CODEFLOW | SlotFlag_TYPE_HIERARCHICAL | SlotFlag_TYPE_VALUE,
    };
}