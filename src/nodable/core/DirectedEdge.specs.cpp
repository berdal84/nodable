#include <gtest/gtest.h>
#include "DirectedEdge.h"
#include "Slot.h"

using namespace ndbl;

TEST( DirectedEdgeUtil, sanitize_edge_CHILD_PARENT)
{
    // Expect no changes on the edge
    {
        // prepare
        Slot output{0, ID<Node>{1}};
        Slot input{0, ID<Node>{2}};
        DirectedEdge edge{input, Relation::CHILD_PARENT, output};
        DirectedEdge copy = edge;
        // act
        DirectedEdgeUtil::sanitize_edge(edge);
        // verify
        EXPECT_EQ( edge, copy);
    }

    // secondary Relation, expect a swap
    {
        // prepare
        Slot output{0, ID<Node>{1}};
        Slot input{0, ID<Node>{2}};
        DirectedEdge edge{input, Relation::PARENT_CHILD, output};
        DirectedEdge copy = edge;
        // act
        DirectedEdgeUtil::sanitize_edge(edge);
        // verity
        EXPECT_NE( edge, copy);
        EXPECT_EQ( edge.relation, Relation::CHILD_PARENT );
        EXPECT_EQ( edge.tail, copy.head );
        EXPECT_EQ( edge.head, copy.tail );
    }
}

