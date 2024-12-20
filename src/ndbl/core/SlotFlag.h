#pragma once
#include "tools/core/types.h"

namespace ndbl
{
    // Nature of the connection allowed by a given Slot
    typedef int SlotFlags;

    //
    // primary  ---->  secondary
    //-------------------------
    // OUTPUT   ----> INPUT
    // FLOW_OUT ----> FLOW_IN
    //
    enum SlotFlag : int
    {
        SlotFlag_NONE          = 0,

        SlotFlag_ORDER_1ST     = 1 << 0,
        SlotFlag_ORDER_2ND     = 1 << 1,

        SlotFlag_TYPE_VALUE    = 1 << 2,
        SlotFlag_TYPE_FLOW     = 1 << 4,

        SlotFlag_IS_THIS       = 1 << 5,
        SlotFlag_NOT_FULL      = 1 << 6,

        SlotFlag_OUTPUT        = SlotFlag_TYPE_VALUE | SlotFlag_ORDER_1ST,
        SlotFlag_INPUT         = SlotFlag_TYPE_VALUE | SlotFlag_ORDER_2ND,
        SlotFlag_FLOW_OUT      = SlotFlag_TYPE_FLOW  | SlotFlag_ORDER_1ST,
        SlotFlag_FLOW_IN       = SlotFlag_TYPE_FLOW  | SlotFlag_ORDER_2ND,

        SlotFlag_FREE_INPUT    = SlotFlag_INPUT      | SlotFlag_NOT_FULL,
        SlotFlag_FREE_FLOW_OUT = SlotFlag_FLOW_OUT   | SlotFlag_NOT_FULL,

        SlotFlag_ORDER_MASK    = SlotFlag_ORDER_1ST  | SlotFlag_ORDER_2ND,
        SlotFlag_TYPE_MASK     = SlotFlag_TYPE_FLOW  | SlotFlag_TYPE_VALUE,
    };

    static SlotFlags switch_order(SlotFlags flags)
    {
        return (i8_t)(flags ^ SlotFlag_ORDER_MASK);
    }
}