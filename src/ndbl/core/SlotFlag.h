#pragma once
#include "tools/core/types.h"

namespace ndbl
{
    // Nature of the connection allowed by a given Slot
    typedef int SlotFlags;
    enum SlotFlag : int
    {
        SlotFlag_NONE              = 0,

        SlotFlag_ORDER_FIRST       = 1 << 0,
        SlotFlag_ORDER_SECOND      = 1 << 1,

        SlotFlag_TYPE_VALUE        = 1 << 2,
        SlotFlag_TYPE_HIERARCHICAL = 1 << 3,
        SlotFlag_TYPE_CODEFLOW     = 1 << 4,

        SlotFlag_IS_THIS           = 1 << 5,
        SlotFlag_NOT_FULL          = 1 << 6,
                                                                                         // primary ----> secondary
                                                                                         //-------------------------
        SlotFlag_OUTPUT            = SlotFlag_TYPE_VALUE        | SlotFlag_ORDER_FIRST,  // output  ---->  ..
        SlotFlag_INPUT             = SlotFlag_TYPE_VALUE        | SlotFlag_ORDER_SECOND, //   ..    ----> input
        SlotFlag_CHILD             = SlotFlag_TYPE_HIERARCHICAL | SlotFlag_ORDER_FIRST,  //  child  ---->  ..
        SlotFlag_PARENT            = SlotFlag_TYPE_HIERARCHICAL | SlotFlag_ORDER_SECOND, //   ..    ----> parent
        SlotFlag_NEXT              = SlotFlag_TYPE_CODEFLOW     | SlotFlag_ORDER_FIRST,  //  next   ---->  ..
        SlotFlag_PREV              = SlotFlag_TYPE_CODEFLOW     | SlotFlag_ORDER_SECOND, //   ..    ----> prev

        SlotFlag_FREE_NEXT         = SlotFlag_NEXT | SlotFlag_NOT_FULL,
        SlotFlag_FREE_CHILD        = SlotFlag_CHILD | SlotFlag_NOT_FULL,

        SlotFlag_ORDER_MASK        = SlotFlag_ORDER_SECOND | SlotFlag_ORDER_FIRST,
        SlotFlag_TYPE_MASK         = SlotFlag_TYPE_CODEFLOW | SlotFlag_TYPE_HIERARCHICAL | SlotFlag_TYPE_VALUE,
    };

    static SlotFlags get_complementary_flags(SlotFlags flags)
    {
        return (i8_t)(flags ^ SlotFlag_ORDER_MASK);
    }
}