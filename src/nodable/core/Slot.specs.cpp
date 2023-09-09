#include <gtest/gtest.h>
#include "DirectedEdge.h"
#include "Slot.h"

using namespace ndbl;

TEST(Slot, operator_equal)
{
    Slot a, b;
    EXPECT_TRUE(a == b);

    Slot c;
    c.capacity = 2;
    EXPECT_TRUE(a == c);

    Slot d;
    d.index = 1;
    EXPECT_FALSE(a == d);

    Slot e;
    e.edges.push_back({});
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
    c.capacity = 2;
    EXPECT_FALSE(a != c);

    Slot d;
    d.index = 1;
    EXPECT_TRUE(a != d);

    Slot e;
    e.node = 1;
    EXPECT_TRUE(a != e);

    Slot f;
    f.way = Way::InOut;
    EXPECT_TRUE(a != f);

    Slot g;
    g.property = 42;
    EXPECT_TRUE(a != g);

    Slot h;
    h.edges.push_back({});
    EXPECT_FALSE(a != h);
}

TEST(Slot, is_full)
{
    Slot slot;

    EXPECT_TRUE(slot.is_full());

    slot.capacity = 2;
    slot.edges.push_back({});
    EXPECT_FALSE(slot.is_full());

    slot.edges.push_back({});
    EXPECT_TRUE(slot.is_full());
}

TEST(Slot, adjacent_slot_at)
{
    // prepare
    Slot slot  {1, ID<Node>{1}};
    slot.capacity = 2;

    Slot slot_0{1, ID<Node>{2}};
    Slot slot_1{1, ID<Node>{3}};

    slot.add_edge({slot, PARENT_CHILD, slot_0});
    slot.add_edge({slot_1, CHILD_PARENT, slot}); //

    // act
    Slot adjacent_slot_0 = slot.adjacent_slot_at(0);
    Slot adjacent_slot_1 = slot.adjacent_slot_at(1);

    // verify
    EXPECT_EQ(adjacent_slot_0, slot_0);
    EXPECT_EQ(adjacent_slot_1, slot_1);
}

TEST(Slot, allows_relation)
{
    // prepare
    Slot slot;

    EXPECT_TRUE( slot.allowed_relation.empty() ); // By default, allows any relation
    EXPECT_TRUE( slot.allows(Relation::PARENT_CHILD) ); // By default, allows any relation
    EXPECT_TRUE( slot.allows(Relation::CHILD_PARENT) ); // By default, allows any relation

    slot.allowed_relation.insert(Relation::PARENT_CHILD);

    EXPECT_TRUE( slot.allows(Relation::PARENT_CHILD) ); // By default, allows any relation
    EXPECT_TRUE( slot.allows(Relation::CHILD_PARENT) ); // By default, allows any relation
}