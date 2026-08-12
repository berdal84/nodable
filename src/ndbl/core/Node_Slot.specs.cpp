#include <gtest/gtest.h>
#include "Node_Slot.h"
#include "core/Flags.h"

using namespace ndbl;

TEST(Node_Slot, default_flags)
{
    Node_Slot slot;
    EXPECT_EQ(slot.flags, Node_Slot::Flag_NONE);
}

TEST(Node_Slot, default_capacity)
{
    Node_Slot slot;
    EXPECT_TRUE(slot.capacity == slot.adjacent.capacity());
}

TEST(Node_Slot, is_full)
{
    Node_Slot out { Node_Slot::Flag_OUTPUT, 2 }; // <-- limit 2
    Node_Slot in1 { Node_Slot::Flag_INPUT     };
    Node_Slot in2 { Node_Slot::Flag_INPUT     };

    EXPECT_FALSE(out.is_full());

    node_slot_add_adjacent(&out, &in1);
    EXPECT_FALSE(out.is_full());

    node_slot_add_adjacent(&out, &in2);
    EXPECT_TRUE(out.is_full());
}

TEST(Node_Slot, adjacent_at)
{
    // prepare
    Node_Slot out { Node_Slot::Flag_OUTPUT, 2};
    Node_Slot in1 { Node_Slot::Flag_INPUT };
    Node_Slot in2 { Node_Slot::Flag_INPUT };

    node_slot_add_adjacent(&out, &in1);
    node_slot_add_adjacent(&out, &in2);

    // act
    Node_Slot* adjacent_slot_0 = out.adjacent[0];
    Node_Slot* adjacent_slot_1 = out.adjacent[1];

    // verify
    EXPECT_EQ(adjacent_slot_0, &in1);
    EXPECT_EQ(adjacent_slot_1, &in2);
}

TEST(Node_Slot, allows_relation)
{
    Node_Slot slot;

    // act
    SET_FLAGS(slot.flags, Node_Slot::Flag_INPUT );

    // verfy
    EXPECT_TRUE( HAS_FLAGS(slot.flags, Node_Slot::Flag_TYPE_VALUE ) );
    EXPECT_TRUE( HAS_FLAGS(slot.flags, Node_Slot::Flag_ORDER_2ND ) );
    EXPECT_TRUE( HAS_FLAGS(slot.flags, Node_Slot::Flag_INPUT ) ); // is TYPE_VALUE + ORDER_2ND
}