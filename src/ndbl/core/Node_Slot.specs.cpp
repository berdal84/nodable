#include <gtest/gtest.h>
#include "Node_Slot.h"
#include "ndbl/core/Flags.h"

#include "ndbl/core/reflection/index.h"
#include "ndbl/core/Log.h"
#include "test/fixtures/basic_test.h"

using namespace ndbl;

typedef ::testing::Basic_Test Node_Slot_;

TEST_F(Node_Slot_, default_flags)
{
    Node_Slot slot;
    node_slot_init(&slot);
    EXPECT_EQ(slot.flags, Node_Slot::Flag_NONE);
}

TEST_F(Node_Slot_, default_capacity)
{
    Node_Slot slot;
    node_slot_init(&slot);
    EXPECT_TRUE(slot.capacity == slot.adjacent.capacity());
}

TEST_F(Node_Slot_, is_full)
{
    Node_Slot out;
    node_slot_init(&out, Node_Slot::Flag_OUTPUT, 2);

    Node_Slot in1;
    node_slot_init(&in1, Node_Slot::Flag_INPUT);

    Node_Slot in2;
    node_slot_init(&in2, Node_Slot::Flag_INPUT);

    EXPECT_FALSE(out.is_full());

    node_slot_add_adjacent(&out, &in1);
    EXPECT_FALSE(out.is_full());

    node_slot_add_adjacent(&out, &in2);
    EXPECT_TRUE(out.is_full());
}

TEST_F(Node_Slot_, adjacent_at)
{
    // prepare
    Node_Slot out;
    node_slot_init(&out, Node_Slot::Flag_OUTPUT, 2);

    Node_Slot in1;
    node_slot_init(&in1, Node_Slot::Flag_INPUT);
    
    Node_Slot in2;
    node_slot_init(&in2, Node_Slot::Flag_INPUT);

    node_slot_add_adjacent(&out, &in1);
    node_slot_add_adjacent(&out, &in2);

    // act
    Node_Slot* adjacent_slot_0 = out.adjacent[0];
    Node_Slot* adjacent_slot_1 = out.adjacent[1];

    // verify
    EXPECT_EQ(adjacent_slot_0, &in1);
    EXPECT_EQ(adjacent_slot_1, &in2);
}

TEST_F(Node_Slot_, allows_relation)
{
    Node_Slot slot;
    node_slot_init(&slot);

    // act
    SET_FLAGS(slot.flags, Node_Slot::Flag_INPUT );

    // verfy
    EXPECT_TRUE( HAS_FLAGS(slot.flags, Node_Slot::Flag_TYPE_VALUE ) );
    EXPECT_TRUE( HAS_FLAGS(slot.flags, Node_Slot::Flag_ORDER_2ND ) );
    EXPECT_TRUE( HAS_FLAGS(slot.flags, Node_Slot::Flag_INPUT ) ); // is TYPE_VALUE + ORDER_2ND
}