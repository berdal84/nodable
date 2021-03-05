#include "gtest/gtest.h"
#include "Core/Member.h"
#include "Node/Node.h"
#include "Node/GraphNode.h"
#include "Node/InstructionNode.h"
#include "Core/Wire.h"
#include "Language/Nodable/NodableLanguage.h"

using namespace Nodable;

TEST( GraphNode, connect)
{
    auto language = std::make_unique<NodableLanguage>();
    auto graph = std::make_unique<GraphNode>(language.get());

    auto node1 = std::make_unique<Node>();
    node1->add("output");

    auto node2 = std::make_unique<Node>();
    node2->add("input");

    auto wire = graph->connect(
            node1->get("output"),
            node2->get("input"));

    EXPECT_EQ(wire->getSource() , node1->get("output"));
    EXPECT_EQ(wire->getTarget() , node2->get("input"));
    EXPECT_EQ(graph->getWireRegistry().size(), 1);
 }

TEST( GraphNode, disconnect)
{
    auto language = std::make_unique<NodableLanguage>();
    auto graph = std::make_unique<GraphNode>(language.get());

    auto a = std::make_unique<Node>();
    a->add("output");

    auto b = std::make_unique<Node>();
    b->add("input");

    EXPECT_EQ(graph->getWireRegistry().size(), 0);

    auto wire = graph->connect(a->get("output"), b->get("input"));

    EXPECT_EQ(graph->getWireRegistry().size(), 1); // wire must be registered when connected

    graph->disconnect(wire);

    EXPECT_EQ(graph->getWires().size(), 0); // wire must be unregistered when disconnected
    EXPECT_EQ(a->getOutputWireCount(), 0);
    EXPECT_EQ(b->getInputWireCount(), 0);
}

TEST( GraphNode, clear)
{
    // prepare
    auto language        = std::make_unique<NodableLanguage>();
    auto graph           = std::make_unique<GraphNode>(language.get());
    auto instructionNode = graph->newInstruction()->as<InstructionNode>();

    auto operatorNode = graph->newOperator(language->findOperator("+"));
    operatorNode->set("rvalue", 2);
    operatorNode->set("lvalue", 2);

    graph->connect(operatorNode->get("result"), instructionNode->value() );

    EXPECT_TRUE(graph->getWireRegistry().size() != 0);
    EXPECT_TRUE(graph->getNodeRegistry().size() != 0);

    // act
    graph->clear();

    // test
    EXPECT_EQ(graph->getWireRegistry().size(), 0);
    EXPECT_EQ(graph->getNodeRegistry().size(), 0);
}