#include "gtest/gtest.h"
#include "Member.h"
#include "Node.h"
#include "GraphNode.h"
#include "InstructionNode.h"
#include "VariableNode.h"
#include "ProgramNode.h"
#include "HeadlessNodeFactory.h"
#include "Wire.h"
#include "NodableLanguage.h"

using namespace Nodable;

TEST( GraphNode, connect)
{
    NodableLanguage language;
    HeadlessNodeFactory factory(&language);
    GraphNode graph(&language, &factory);

    auto node1 = graph.newNode();
    node1->getProps()->add("output", Nodable::Visibility::Default, Type_Boolean, Way_Default);

    auto node2  = graph.newNode();
    node2->getProps()->add("input", Nodable::Visibility::Default, Type_Boolean, Way_Default);

    auto wire = graph.connect(
            node1->getProps()->get("output"),
            node2->getProps()->get("input"));

    EXPECT_EQ(wire->getSource() , node1->getProps()->get("output"));
    EXPECT_EQ(wire->getTarget() , node2->getProps()->get("input"));
    EXPECT_EQ(graph.getWireRegistry().size(), 1);
 }

TEST( GraphNode, disconnect)
{
    NodableLanguage language;
    HeadlessNodeFactory factory(&language);
    GraphNode graph(&language, &factory);

    auto a = graph.newNode();
    auto output = a->getProps()->add("output", Nodable::Visibility::Default, Type_Boolean, Way_Default);

    auto b = graph.newNode();
    auto input = b->getProps()->add("input", Nodable::Visibility::Default, Type_Boolean, Way_Default);

    EXPECT_EQ(graph.getWireRegistry().size(), 0);
    EXPECT_EQ(graph.getRelationRegistry().size(), 0);

    auto wire = graph.connect(output, input);

    EXPECT_EQ(graph.getWireRegistry().size(), 1); // wire must be registered when connected
    EXPECT_EQ(graph.getRelationRegistry().size(), 1); // relation must be registered when connected

    graph.disconnect(wire);

    EXPECT_EQ(graph.getWires().size(), 0); // wire must be unregistered when disconnected
    EXPECT_EQ(a->getOutputWireCount(), 0);
    EXPECT_EQ(b->getInputWireCount(), 0);
}

TEST( GraphNode, clear)
{
    // prepare
    NodableLanguage language;
    HeadlessNodeFactory factory(&language);
    GraphNode graph(&language, &factory);
    InstructionNode* instructionNode = graph.newInstruction();

    Node* operatorNode = graph.newOperator(language.findOperator("+"));
    auto props = operatorNode->getProps();
    props->get("rvalue")->set(2);
    props->get("lvalue")->set(2);

    graph.connect(props->get("result"), instructionNode->getValue() );

    EXPECT_TRUE(graph.getWireRegistry().size() != 0);
    EXPECT_TRUE(graph.getNodeRegistry().size() != 0);
    EXPECT_TRUE(graph.getRelationRegistry().size() != 0);

    // act
    graph.clear();

    // test
    EXPECT_EQ(graph.getWireRegistry().size(), 0);
    EXPECT_EQ(graph.getNodeRegistry().size(), 0);
    EXPECT_EQ(graph.getRelationRegistry().size(), 0);

}


TEST( GraphNode, create_and_delete_relations)
{
    // prepare
    NodableLanguage language;
    HeadlessNodeFactory factory(&language);
    GraphNode graph(&language, &factory);
    ScopedCodeBlockNode* program = graph.getProgram();
    EXPECT_EQ(graph.getRelationRegistry().size(), 0);
    Node* n1 = graph.newVariable(Type_Any, "n1", program);
    EXPECT_EQ(graph.getRelationRegistry().size(), 0);
    Node* n2 = graph.newVariable(Type_Double, "n2", program);

    // Act and test

    // is child of (and by reciprocity "is parent of")
    EXPECT_EQ(graph.getRelationRegistry().size(), 0);
    EXPECT_EQ(n2->getChildren().size(), 0);
    graph.connect(n1, n2, RelationType::IS_CHILD_OF, false);
    EXPECT_EQ(n2->getChildren().size(), 1);
    EXPECT_EQ(graph.getRelationRegistry().size(), 1);
    graph.disconnect(n1, n2, RelationType::IS_CHILD_OF);
    EXPECT_EQ(n2->getChildren().size(), 0);

    // Is input of
    EXPECT_EQ(graph.getRelationRegistry().size(), 0);
    EXPECT_EQ(n2->getInputs().size(), 0);
    graph.connect(n1, n2, RelationType::IS_INPUT_OF, false);
    EXPECT_EQ(n2->getInputs().size(), 1);
    EXPECT_EQ(graph.getRelationRegistry().size(), 1);
    graph.disconnect(n1, n2, RelationType::IS_INPUT_OF);
    EXPECT_EQ(n2->getInputs().size(), 0);
    EXPECT_EQ(graph.getRelationRegistry().size(), 0);
}
