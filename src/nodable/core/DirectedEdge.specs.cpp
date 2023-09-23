#include "DirectedEdge.h"
#include "NodeFactory.h"
#include "Slot.h"
#include "core/fixtures/core.h"
#include <gtest/gtest.h>

using namespace ndbl;
using namespace ndbl;
typedef ::testing::Core DirectedEdge_;

TEST_F( DirectedEdge_, normalize)
{
    PoolID<Node> node_1 = factory.create_node();
    PoolID<Node> node_2 = factory.create_node();

    // Expect no changes on the edge
    {
        // prepare
        DirectedEdge edge{
                *node_1->find_slot( THIS_PROPERTY, SlotFlag_PREV ),
                *node_2->find_slot( THIS_PROPERTY, SlotFlag_NEXT )};
        DirectedEdge copy = edge;
        // act
        DirectedEdge::normalize(edge);
        // verify
        EXPECT_EQ( edge, copy);
    }

    // secondary Relation, expect a swap
    {
        // prepare
        DirectedEdge edge{
                *node_1->find_slot( THIS_PROPERTY, SlotFlag_NEXT ),
                *node_2->find_slot( THIS_PROPERTY, SlotFlag_PREV )};
        DirectedEdge copy = edge;
        // act
        DirectedEdge::normalize(edge);
        // verity
        EXPECT_NE( edge, copy);
        EXPECT_EQ( edge.tail, copy.head );
        EXPECT_EQ( edge.head, copy.tail );
    }
}

