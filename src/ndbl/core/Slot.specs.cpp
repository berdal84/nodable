#include <gtest/gtest.h>
#include "DirectedEdge.h"
#include "Slot.h"

using namespace ndbl;

TEST(Slot, is_full)
{
    Slot slot;
    Slot other1;
    Slot other2;

    EXPECT_TRUE(slot.is_full());

    slot.expand_capacity(2);
    slot.add_adjacent(&other1);
    EXPECT_FALSE(slot.is_full());

    slot.add_adjacent(&other2);
    EXPECT_TRUE(slot.is_full());
}

TEST(Slot, adjacent_at)
{
    // prepare
    Slot slot  {nullptr, SlotFlag_OUTPUT};
    slot.expand_capacity(2);

    Slot slot_0{nullptr, SlotFlag_INPUT};
    Slot slot_1{nullptr, SlotFlag_INPUT};

    slot.add_adjacent(&slot_0);
    slot.add_adjacent(&slot_1);

    // act
    Slot* adjacent_slot_0 = slot.adjacent_at( 0 );
    Slot* adjacent_slot_1 = slot.adjacent_at( 1 );

    // verify
    EXPECT_EQ(adjacent_slot_0, &slot_0);
    EXPECT_EQ(adjacent_slot_1, &slot_1);
}

TEST(Slot, allows_relation)
{
    // prepare
    Slot slot;

    EXPECT_TRUE(slot.flags() == SlotFlag_NONE );

    slot.set_flags( SlotFlag_INPUT );

    EXPECT_TRUE( SlotFlag_INPUT & SlotFlag_TYPE_VALUE );
    EXPECT_TRUE( slot.has_flags( SlotFlag_TYPE_VALUE ) );
}