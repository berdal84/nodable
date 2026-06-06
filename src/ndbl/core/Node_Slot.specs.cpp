#include <gtest/gtest.h>
#include "Node_Slot_Link.h"
#include "Node_Slot.h"

using namespace ndbl;

TEST(Node_Slot, default_flags)
{
    Node_Slot slot;
    EXPECT_EQ(slot.flags(), Node_Slot_Flag_NONE);
}

TEST(Node_Slot, default_capacity)
{
    Node_Slot slot;
    EXPECT_TRUE(slot.capacity() == slot._adjacent.capacity());
}

TEST(Node_Slot, is_full)
{
    Node_Slot out { Node_Slot_Flag_OUTPUT, 2 }; // <-- limit 2
    Node_Slot in1 { Node_Slot_Flag_INPUT     };
    Node_Slot in2 { Node_Slot_Flag_INPUT     };

    EXPECT_FALSE(out.is_full());

    out.add_adjacent(&in1);
    EXPECT_FALSE(out.is_full());

    out.add_adjacent(&in2);
    EXPECT_TRUE(out.is_full());
}

TEST(Node_Slot, adjacent_at)
{
    // prepare
    Node_Slot out { Node_Slot_Flag_OUTPUT, 2};
    Node_Slot in1 { Node_Slot_Flag_INPUT };
    Node_Slot in2 { Node_Slot_Flag_INPUT };

    out.add_adjacent(&in1);
    out.add_adjacent(&in2);

    // act
    Node_Slot* adjacent_slot_0 = out.adjacent_at(0 );
    Node_Slot* adjacent_slot_1 = out.adjacent_at(1 );

    // verify
    EXPECT_EQ(adjacent_slot_0, &in1);
    EXPECT_EQ(adjacent_slot_1, &in2);
}

TEST(Node_Slot, allows_relation)
{
    Node_Slot slot;

    // act
    slot.set_flags( Node_Slot_Flag_INPUT );

    // verfy
    EXPECT_TRUE( slot.has_flags( Node_Slot_Flag_TYPE_VALUE ) );
    EXPECT_TRUE( slot.has_flags( Node_Slot_Flag_ORDER_2ND ) );
    EXPECT_TRUE( slot.has_flags( Node_Slot_Flag_INPUT ) ); // is TYPE_VALUE + ORDER_2ND
}