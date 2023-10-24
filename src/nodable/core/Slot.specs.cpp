#include <gtest/gtest.h>
#include "DirectedEdge.h"
#include "Slot.h"

using namespace ndbl;

TEST(Slot, operator_equal)
{
    Slot a;
    Slot b;
    EXPECT_TRUE(a == b);

    Slot c;
    c.expand_capacity(2);
    EXPECT_TRUE(a == c);

    Slot d;
    d.id.reset(1);
    EXPECT_FALSE(a == d);

    Slot e;
    e.expand_capacity( 1 );
    e.add_adjacent({});
    EXPECT_TRUE(a == e);
}

TEST(Slot, default_constructor_produce_null)
{
    Slot slot;
    EXPECT_EQ(slot, Slot::null);
}

TEST(Slot, operator_non_equal)
{
    Slot a, b;
    EXPECT_FALSE(a != b);

    Slot c;
    c.expand_capacity(2);
    EXPECT_FALSE(a != c);

    Slot d;
    d.id.reset(1);
    EXPECT_TRUE(a != d);

    Slot e;
    e.node = PoolID<Node>{1};
    EXPECT_TRUE(a != e);

    Slot f;
    f.flags |= SlotFlag_ORDER_FIRST;
    EXPECT_TRUE(a != f);

    Slot g;
    g.property.reset(42);
    EXPECT_TRUE(a != g);

    Slot h;
    h.expand_capacity(1);
    h.add_adjacent({});
    EXPECT_FALSE(a != h);
}

TEST(Slot, is_full)
{
    Slot slot;

    EXPECT_TRUE(slot.is_full());

    slot.expand_capacity(2);
    slot.add_adjacent({});
    EXPECT_FALSE(slot.is_full());

    slot.add_adjacent({});
    EXPECT_TRUE(slot.is_full());
}

TEST(Slot, adjacent_at)
{
    // prepare
    Slot slot  {1, PoolID<Node>{1}, SlotFlag_PARENT};
    slot.expand_capacity(2);

    Slot slot_0{2, PoolID<Node>{2}, SlotFlag_CHILD};
    Slot slot_1{3, PoolID<Node>{3}, SlotFlag_CHILD};

    slot.add_adjacent( slot_0 );
    slot.add_adjacent( slot_1 );

    // act
    SlotRef adjacent_slot_0 = slot.adjacent_at( 0 );
    SlotRef adjacent_slot_1 = slot.adjacent_at( 1 );

    // verify
    EXPECT_EQ(adjacent_slot_0, slot_0);
    EXPECT_EQ(adjacent_slot_1, slot_1);
}

TEST(Slot, allows_relation)
{
    // prepare
    Slot slot;

    EXPECT_TRUE( slot.flags == SlotFlag_NONE );

    slot.allow( SlotFlag_CHILD );

    EXPECT_TRUE( slot.flags & SlotFlag_TYPE_HIERARCHICAL );
}