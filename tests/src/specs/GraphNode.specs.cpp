#include <gtest/gtest.h>

#include <nodable/core/Member.h>
#include <nodable/core/Node.h>
#include <nodable/core/GraphNode.h>
#include <nodable/core/InstructionNode.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/HeadlessNodeFactory.h>
#include <nodable/core/Wire.h>
#include <nodable/core/languages/Nodable.h>
#include <nodable/core/InvokableComponent.h>
#include <nodable/core/Scope.h>

using namespace Nodable;
using namespace Nodable::R;

TEST( GraphNode, connect)
{
    LanguageNodable language;
    HeadlessNodeFactory factory(&language);
    bool autocompletion = false;
    GraphNode graph(&language, &factory, &autocompletion);

    auto node1 = graph.create_node();
    node1->props()->add("output", Visibility::Default, R::get_meta_type<bool>(), Way_Default);

    auto node2  = graph.create_node();
    node2->props()->add("input", Visibility::Default, R::get_meta_type<bool>(), Way_Default);

    auto wire = graph.connect(
            node1->props()->get("output"),
            node2->props()->get("input"));

    EXPECT_EQ(wire->members.src , node1->props()->get("output"));
    EXPECT_EQ(wire->members.dst , node2->props()->get("input"));
    EXPECT_EQ(graph.get_wire_registry().size(), 1);
 }

TEST( GraphNode, disconnect)
{
    LanguageNodable language;
    HeadlessNodeFactory factory(&language);
    bool autocompletion  = false;
    GraphNode graph(&language, &factory,  &autocompletion);

    auto a = graph.create_node();
    auto output = a->props()->add("output", Visibility::Default, R::get_meta_type<bool>(), Way_Default);

    auto b = graph.create_node();
    auto input = b->props()->add("input", Visibility::Default, R::get_meta_type<bool>(), Way_Default);

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
    bool autocompletion  = false;
    GraphNode graph(&language, &factory,  &autocompletion);
    InstructionNode* instructionNode = graph.create_instr();

    auto ope = language.findOperator("+");
    EXPECT_TRUE(ope != nullptr);
    Node* operatorNode = graph.create_operator(ope);
    auto props = operatorNode->props();
    props->get(k_lh_value_member_name)->set(2);
    props->get(k_rh_value_member_name)->set(2);

    graph.connect(props->get(k_this_member_name), instructionNode->get_root_node_member() );

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
    bool autocompletion  = false;
    GraphNode graph(&language, &factory, &autocompletion);
    Node* program = graph.create_root();
    EXPECT_EQ(graph.get_relation_registry().size(), 0);
    Node* n1 = graph.create_variable(R::get_meta_type<double>(), "n1", program->get<Scope>());
    EXPECT_EQ(graph.get_relation_registry().size(), 0);
    Node* n2 = graph.create_variable(R::get_meta_type<double>(), "n2", program->get<Scope>());

    // Act and test

    // is child of (and by reciprocity "is parent of")
    EXPECT_EQ(graph.get_relation_registry().size(), 0);
    EXPECT_EQ(n2->children_slots().size(), 0);
    graph.connect({n1, EdgeType::IS_CHILD_OF, n2}, false);
    EXPECT_EQ(n2->children_slots().size(), 1);
    EXPECT_EQ(graph.get_relation_registry().size(), 1);
    graph.disconnect({n1, EdgeType::IS_CHILD_OF, n2});
    EXPECT_EQ(n2->children_slots().size(), 0);

    // Is input of
    EXPECT_EQ(graph.get_relation_registry().size(), 0);
    EXPECT_EQ(n2->input_slots().size(), 0);
    graph.connect({n1, EdgeType::IS_INPUT_OF, n2}, false);
    EXPECT_EQ(n2->input_slots().size(), 1);
    EXPECT_EQ(graph.get_relation_registry().size(), 1);
    graph.disconnect({n1, EdgeType::IS_INPUT_OF, n2});
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
    bool autocompletion  = false;
    GraphNode graph(&language, &factory, &autocompletion);
    Node* program = graph.create_root();

    // create b
    auto b = graph.create_variable(R::get_meta_type<double>(), "b", program->get<Scope>());
    b->set(6.0);

    // create assign operator
    FunctionSignature signature("operator=");
    signature.set_return_type(R::get_meta_type<double>());
    signature.push_args(R::get_meta_type<double &>(), R::get_meta_type<double>());
    auto assign = graph.create_operator(language.findOperator(&signature));
    auto op = assign->get<InvokableComponent>();

    // connect b and assign
    graph.connect(b->get_value(), assign->props()->get(k_lh_value_member_name) );

    op->get_r_handed_val()->set(5.0);

    ASSERT_DOUBLE_EQ(b->get_value()->convert_to<double>(), 6.0 );
    ASSERT_DOUBLE_EQ(assign->props()->get(k_lh_value_member_name)->convert_to<double>(), 6.0 );
    ASSERT_DOUBLE_EQ(assign->props()->get(k_rh_value_member_name)->convert_to<double>(), 5.0 );
    ASSERT_DOUBLE_EQ(assign->props()->get(k_value_member_name)->convert_to<double>(), 0.0 );

    // apply
    assign->eval();

    ASSERT_DOUBLE_EQ(b->get_value()->convert_to<double>(), 5.0 );
    ASSERT_DOUBLE_EQ(assign->props()->get(k_lh_value_member_name)->convert_to<double>(), 5.0 );
    ASSERT_DOUBLE_EQ(assign->props()->get(k_rh_value_member_name)->convert_to<double>(), 5.0 );
    ASSERT_DOUBLE_EQ(assign->props()->get(k_value_member_name)->convert_to<double>(), 5.0 );
}