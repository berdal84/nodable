#include "gtest/gtest.h"
#include <Core/Member.h>
#include <Node/Node.h>
#include <Core/Wire.h>

using namespace Nodable;

TEST( Wire, Connect_two_nodes_with_a_wire)
{
    std::unique_ptr<Node> a(new Node);
    a->add("output");

    std::unique_ptr<Node> b(new Node);
    b->add("input");

    auto wire = Node::Connect(a->get("output"), b->get("input"));

    EXPECT_EQ(wire->getSource() , a->get("output"));
    EXPECT_EQ(wire->getTarget() , b->get("input"));

    Node::Disconnect(wire);
}

TEST( Wire, Disconnect)
{

    Node a;
    a.add("output");

    Node b;
    b.add("input");

    auto wire = Node::Connect(a.get("output"), b.get("input"));

    Node::Disconnect(wire);

    EXPECT_EQ(a.getOutputWireCount(), 0);
    EXPECT_EQ(b.getInputWireCount(), 0);
}