#include <gtest/gtest.h>

#include <nodable/Member.h>
#include <nodable/Node.h>
#include <nodable/GraphNode.h>
#include <nodable/InstructionNode.h>
#include <nodable/VariableNode.h>
#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/HeadlessNodeFactory.h>
#include <nodable/Wire.h>
#include <nodable/LanguageNodable.h>
#include <nodable/InvokableComponent.h>

using namespace Nodable;

TEST( GraphNode, connect)
{
    LanguageNodable language;
    HeadlessNodeFactory factory(&language);
    GraphNode graph(&language, &factory);

    auto node1 = graph.newNode();
    node1->getProps()->add("output", Visibility::Default, Type_Boolean, Way_Default);

    auto node2  = graph.newNode();
    node2->getProps()->add("input", Visibility::Default, Type_Boolean, Way_Default);

    auto wire = graph.connect(
            node1->getProps()->get("output"),
            node2->getProps()->get("input"));

    EXPECT_EQ(wire->getSource() , node1->getProps()->get("output"));
    EXPECT_EQ(wire->getTarget() , node2->getProps()->get("input"));
    EXPECT_EQ(graph.getWireRegistry().size(), 1);
 }

TEST( GraphNode, disconnect)
{
    LanguageNodable language;
    HeadlessNodeFactory factory(&language);
    GraphNode graph(&language, &factory);

    auto a = graph.newNode();
    auto output = a->getProps()->add("output", Visibility::Default, Type_Boolean, Way_Default);

    auto b = graph.newNode();
    auto input = b->getProps()->add("input", Visibility::Default, Type_Boolean, Way_Default);

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
    LanguageNodable language;
    HeadlessNodeFactory factory(&language);
    GraphNode graph(&language, &factory);
    InstructionNode* instructionNode = graph.newInstruction();

    auto ope = language.findOperator("+");
    EXPECT_TRUE(ope != nullptr);
    Node* operatorNode = graph.newOperator(ope);
    auto props = operatorNode->getProps();
    props->get("rvalue")->set(2);
    props->get("lvalue")->set(2);

    graph.connect(props->get("result"), instructionNode->value() );

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
    LanguageNodable language;
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
    EXPECT_EQ(n2->get_children().size(), 0);
    graph.connect(n1, n2, RelationType::IS_CHILD_OF, false);
    EXPECT_EQ(n2->get_children().size(), 1);
    EXPECT_EQ(graph.getRelationRegistry().size(), 1);
    graph.disconnect(n1, n2, RelationType::IS_CHILD_OF);
    EXPECT_EQ(n2->get_children().size(), 0);

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

TEST(Graph, by_reference_assign)
{
    // we will create this graph manually
    //            "double b = 6;"
    //            "b = 5;"
    //            "b;";

    // prepare
    LanguageNodable language;
    HeadlessNodeFactory factory(&language);
    GraphNode graph(&language, &factory);
    ScopedCodeBlockNode* program = graph.getProgram();

    // create b
    auto b = graph.newVariable(Type_Double, "b", program);
    b->set(6.0);

    // create assign operator
    FunctionSignature signature("operator=");
    signature.set_return_type(Type_Double);
    signature.push_args(Type_Double_Ref, Type_Double);
    auto assign = graph.newOperator( language.findOperator(&signature) );
    auto op = assign->getComponent<InvokableComponent>();

    // connect b and assign
    graph.connect( b->value(), assign->getProps()->get("lvalue") );

    op->get_r_handed_val()->set(5.0);

    ASSERT_DOUBLE_EQ( b->value()->convert_to<double>(), 6.0 );
    ASSERT_DOUBLE_EQ( assign->getProps()->get("lvalue")->convert_to<double>(), 6.0 );
    ASSERT_DOUBLE_EQ( assign->getProps()->get("rvalue")->convert_to<double>(), 5.0 );
    ASSERT_DOUBLE_EQ( assign->getProps()->get("result")->convert_to<double>(), 0.0 );

    // apply
    assign->eval();

    ASSERT_DOUBLE_EQ( b->value()->convert_to<double>(), 5.0 );
    ASSERT_DOUBLE_EQ( assign->getProps()->get("lvalue")->convert_to<double>(), 5.0 );
    ASSERT_DOUBLE_EQ( assign->getProps()->get("rvalue")->convert_to<double>(), 5.0 );
    ASSERT_DOUBLE_EQ( assign->getProps()->get("result")->convert_to<double>(), 5.0 );
}