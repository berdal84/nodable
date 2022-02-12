#include <gtest/gtest.h>

#include <nodable/Member.h>
#include <nodable/Node.h>
#include <nodable/GraphNode.h>
#include <nodable/InstructionNode.h>
#include <nodable/VariableNode.h>
#include <nodable/HeadlessNodeFactory.h>
#include <nodable/Wire.h>
#include <nodable/LanguageNodable.h>
#include <nodable/InvokableComponent.h>
#include <nodable/Scope.h>

using namespace Nodable;
using namespace Nodable::Reflect;

TEST( GraphNode, connect)
{
    LanguageNodable language;
    HeadlessNodeFactory factory(&language);
    GraphNode graph(&language, &factory);

    auto node1 = graph.create_node();
    node1->props()->add("output", Visibility::Default, Type_Boolean, Way_Default);

    auto node2  = graph.create_node();
    node2->props()->add("input", Visibility::Default, Type_Boolean, Way_Default);

    auto wire = graph.connect(
            node1->props()->get("output"),
            node2->props()->get("input"));

    EXPECT_EQ(wire->getSource() , node1->props()->get("output"));
    EXPECT_EQ(wire->getTarget() , node2->props()->get("input"));
    EXPECT_EQ(graph.get_wire_registry().size(), 1);
 }

TEST( GraphNode, disconnect)
{
    LanguageNodable language;
    HeadlessNodeFactory factory(&language);
    GraphNode graph(&language, &factory);

    auto a = graph.create_node();
    auto output = a->props()->add("output", Visibility::Default, Type_Boolean, Way_Default);

    auto b = graph.create_node();
    auto input = b->props()->add("input", Visibility::Default, Type_Boolean, Way_Default);

    EXPECT_EQ(graph.get_wire_registry().size(), 0);
    EXPECT_EQ(graph.get_relation_registry().size(), 0);

    auto wire = graph.connect(output, input);

    EXPECT_EQ(graph.get_wire_registry().size(), 1); // wire must be registered when connected
    EXPECT_EQ(graph.get_relation_registry().size(), 1); // relation must be registered when connected

    graph.disconnect(wire);

    EXPECT_EQ(graph.get_wires().size(), 0); // wire must be unregistered when disconnected
    EXPECT_EQ(a->get_output_wire_count(), 0);
    EXPECT_EQ(b->get_input_wire_count(), 0);
}

TEST( GraphNode, clear)
{
    // prepare
    LanguageNodable language;
    HeadlessNodeFactory factory(&language);
    GraphNode graph(&language, &factory);
    InstructionNode* instructionNode = graph.create_instr();

    auto ope = language.findOperator("+");
    EXPECT_TRUE(ope != nullptr);
    Node* operatorNode = graph.create_operator(ope);
    auto props = operatorNode->props();
    props->get("rvalue")->set(2);
    props->get("lvalue")->set(2);

    graph.connect(props->get(Node::VALUE_MEMBER_NAME), instructionNode->get_root_node_member() );

    EXPECT_TRUE(graph.get_wire_registry().size() != 0);
    EXPECT_TRUE(graph.get_node_registry().size() != 0);
    EXPECT_TRUE(graph.get_relation_registry().size() != 0);

    // act
    graph.clear();

    // test
    EXPECT_EQ(graph.get_wire_registry().size(), 0);
    EXPECT_EQ(graph.get_node_registry().size(), 0);
    EXPECT_EQ(graph.get_relation_registry().size(), 0);

}


TEST( GraphNode, create_and_delete_relations)
{
    // prepare
    LanguageNodable language;
    HeadlessNodeFactory factory(&language);
    GraphNode graph(&language, &factory);
    Node* program = graph.create_root();
    EXPECT_EQ(graph.get_relation_registry().size(), 0);
    Node* n1 = graph.create_variable(Type_Unknown, "n1", program->get<Scope>());
    EXPECT_EQ(graph.get_relation_registry().size(), 0);
    Node* n2 = graph.create_variable(Type_Double, "n2", program->get<Scope>());

    // Act and test

    // is child of (and by reciprocity "is parent of")
    EXPECT_EQ(graph.get_relation_registry().size(), 0);
    EXPECT_EQ(n2->children_slots().size(), 0);
    graph.connect(n1, n2, Relation_t::IS_CHILD_OF, false);
    EXPECT_EQ(n2->children_slots().size(), 1);
    EXPECT_EQ(graph.get_relation_registry().size(), 1);
    graph.disconnect(n1, n2, Relation_t::IS_CHILD_OF);
    EXPECT_EQ(n2->children_slots().size(), 0);

    // Is input of
    EXPECT_EQ(graph.get_relation_registry().size(), 0);
    EXPECT_EQ(n2->input_slots().size(), 0);
    graph.connect(n1, n2, Relation_t::IS_INPUT_OF, false);
    EXPECT_EQ(n2->input_slots().size(), 1);
    EXPECT_EQ(graph.get_relation_registry().size(), 1);
    graph.disconnect(n1, n2, Relation_t::IS_INPUT_OF);
    EXPECT_EQ(n2->input_slots().size(), 0);
    EXPECT_EQ(graph.get_relation_registry().size(), 0);
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
    Node* program = graph.create_root();

    // create b
    auto b = graph.create_variable(Type_Double, "b", program->get<Scope>());
    b->set(6.0);

    // create assign operator
    FunctionSignature signature("operator=");
    signature.set_return_type(Type_Double);
    signature.push_args(Type_Double_Ref, Type_Double);
    auto assign = graph.create_operator(language.findOperator(&signature));
    auto op = assign->get<InvokableComponent>();

    // connect b and assign
    graph.connect(b->get_value(), assign->props()->get("lvalue") );

    op->get_r_handed_val()->set(5.0);

    ASSERT_DOUBLE_EQ(b->get_value()->convert_to<double>(), 6.0 );
    ASSERT_DOUBLE_EQ(assign->props()->get("lvalue")->convert_to<double>(), 6.0 );
    ASSERT_DOUBLE_EQ(assign->props()->get("rvalue")->convert_to<double>(), 5.0 );
    ASSERT_DOUBLE_EQ(assign->props()->get(Node::VALUE_MEMBER_NAME)->convert_to<double>(), 0.0 );

    // apply
    assign->eval();

    ASSERT_DOUBLE_EQ(b->get_value()->convert_to<double>(), 5.0 );
    ASSERT_DOUBLE_EQ(assign->props()->get("lvalue")->convert_to<double>(), 5.0 );
    ASSERT_DOUBLE_EQ(assign->props()->get("rvalue")->convert_to<double>(), 5.0 );
    ASSERT_DOUBLE_EQ(assign->props()->get(Node::VALUE_MEMBER_NAME)->convert_to<double>(), 5.0 );
}