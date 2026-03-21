#include <gtest/gtest.h>
#include "ASTSlotLink.h"
#include "ASTNodeSlot.h"

using namespace ndbl;

TEST(Slot, default_flags)
{
    ASTNodeSlot slot;
    EXPECT_EQ(slot.flags(), SlotFlag_NONE);
}

TEST(Slot, default_capacity)
{
    ASTNodeSlot slot;
    EXPECT_TRUE(slot.capacity() == slot._adjacent.capacity());
}

TEST(Slot, is_full)
{
    ASTNodeSlot out { SlotFlag_OUTPUT, 2 }; // <-- limit 2
    ASTNodeSlot in1 { SlotFlag_INPUT     };
    ASTNodeSlot in2 { SlotFlag_INPUT     };

    EXPECT_FALSE(out.is_full());

    out.add_adjacent(&in1);
    EXPECT_FALSE(out.is_full());

    out.add_adjacent(&in2);
    EXPECT_TRUE(out.is_full());
}

TEST(Slot, adjacent_at)
{
    // prepare
    ASTNodeSlot out { SlotFlag_OUTPUT, 2};
    ASTNodeSlot in1 { SlotFlag_INPUT };
    ASTNodeSlot in2 { SlotFlag_INPUT };

    out.add_adjacent(&in1);
    out.add_adjacent(&in2);

    // act
    ASTNodeSlot* adjacent_slot_0 = out.adjacent_at(0 );
    ASTNodeSlot* adjacent_slot_1 = out.adjacent_at(1 );

    // verify
    EXPECT_EQ(adjacent_slot_0, &in1);
    EXPECT_EQ(adjacent_slot_1, &in2);
}

TEST(Slot, allows_relation)
{
    ASTNodeSlot slot;

    // act
    slot.set_flags( SlotFlag_INPUT );

    // verfy
    EXPECT_TRUE( slot.has_flags( SlotFlag_TYPE_VALUE ) );
    EXPECT_TRUE( slot.has_flags( SlotFlag_ORDER_2ND ) );
    EXPECT_TRUE( slot.has_flags( SlotFlag_INPUT ) ); // is TYPE_VALUE + ORDER_2ND
}