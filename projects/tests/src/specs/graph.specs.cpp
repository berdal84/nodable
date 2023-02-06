#include <gtest/gtest.h>

#include "../fixtures/core.h"
#include <fw/core/reflection/func_type.h>
#include <ndbl/core/DirectedEdge.h>
#include <ndbl/core/GraphNode.h>
#include <ndbl/core/InstructionNode.h>
#include <ndbl/core/InvokableComponent.h>
#include <ndbl/core/Node.h>
#include <ndbl/core/NodeFactory.h>
#include <ndbl/core/Property.h>
#include <ndbl/core/Scope.h>
#include <ndbl/core/VariableNode.h>
#include <ndbl/core/language/Nodlang.h>

using namespace ndbl;
typedef ::testing::Core Graph;

TEST_F(Graph, connect)
{
    auto node1   = graph.create_node();
    auto node1_output = node1->props()->add<bool>("output");

    auto node2  = graph.create_node();
    auto node2_input  = node2->props()->add<bool>("input");

    auto edge = graph.connect(node1_output.get(), node2_input.get());

    EXPECT_EQ(edge->prop.src , node1_output.get());
    EXPECT_EQ(edge->prop.dst , node2_input.get());
    EXPECT_EQ(graph.get_edge_registry().size(), 1);
 }

TEST_F(Graph, disconnect)
{
    Node* a      = graph.create_node();
    auto  output = a->props()->add<bool>("output");

    Node* b     = graph.create_node();
    auto  input = b->props()->add<bool>("input");

    EXPECT_EQ(graph.get_edge_registry().size(), 0);

    const DirectedEdge* edge = graph.connect(output.get(), input.get());

    EXPECT_EQ(graph.get_edge_registry().size(), 1); // edge must be registered when connected

    graph.disconnect(edge);

    EXPECT_EQ(graph.get_edge_registry().size()  , 0); // edge must be unregistered when disconnected
    EXPECT_EQ(a->outgoing_edge_count(), 0);
    EXPECT_EQ(b->incoming_edge_count() , 0);
}

TEST_F(Graph, clear)
{
    InstructionNode* instructionNode = graph.create_instr();
    fw::func_type*   fct_type        = fw::func_type_builder<int(int, int)>::with_id("+");
    auto             operator_fct    = language.find_operator_fct_exact(fct_type);

    delete fct_type;

    EXPECT_TRUE(operator_fct.get() != nullptr);
    Node* operatorNode = graph.create_operator(operator_fct.get());
    auto props = operatorNode->props();
    props->get(k_lh_value_property_name)->set(2);
    props->get(k_rh_value_property_name)->set(2);

    graph.connect(props->get(k_this_property_name), instructionNode->get_root_node_property() );

    EXPECT_TRUE(graph.get_node_registry().size() != 0);
    EXPECT_TRUE(graph.get_edge_registry().size() != 0);

    // act
    graph.clear();

    // test
    EXPECT_EQ(graph.get_node_registry().size(), 0);
    EXPECT_EQ(graph.get_edge_registry().size(), 0);

}


TEST_F(Graph, create_and_delete_relations)
{
    // prepare
    auto double_type = fw::type::get<double>();
    auto scope       = graph.create_root()->get<Scope>();
    auto& edges      = graph.get_edge_registry();
    EXPECT_EQ(edges.size(), 0);
    Node* n1 = graph.create_variable(double_type, "n1", scope);
    EXPECT_EQ(edges.size(), 0);
    Node* n2 = graph.create_variable(double_type, "n2", scope);

    // Act and test

    // is child of (and by reciprocity "is parent of")
    EXPECT_EQ(edges.size(), 0);
    EXPECT_EQ(n2->children_slots().size(), 0);
    auto edge1 = graph.connect({n1, Edge_t::IS_CHILD_OF, n2}, false);
    EXPECT_EQ(n2->children_slots().size(), 1);
    EXPECT_EQ(edges.size(), 1);
    graph.disconnect(edge1);
    EXPECT_EQ(n2->children_slots().size(), 0);

    // Is input of
    EXPECT_EQ(edges.size(), 0);
    EXPECT_EQ(n2->inputs().size(), 0);
    auto edge2 = graph.connect({n1, Edge_t::IS_INPUT_OF, n2}, false);
    EXPECT_EQ(n2->inputs().size(), 1);
    EXPECT_EQ(edges.size(), 1);
    graph.disconnect(edge2);
    EXPECT_EQ(n2->inputs().size(), 0);
    EXPECT_EQ(edges.size(), 0);
}