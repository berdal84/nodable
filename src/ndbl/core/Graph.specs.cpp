#include <gtest/gtest.h>

#include "tools/core/reflection/Type.h"

#include "Graph.h"
#include "FunctionNode.h"
#include "Node.h"
#include "Scope.h"
#include "DirectedEdge.h"

#include "fixtures/core.h"
#include "Utils.h"

using namespace ndbl;
using namespace tools;
typedef ::testing::Core Graph_;

TEST_F(Graph_, connect)
{
    // Prepare
    Graph* graph = app.get_graph();
    auto* node_1 = graph->create_node();
    auto* prop_1 = node_1->add_prop<bool>("prop_1");
    auto* slot_1 = node_1->add_slot(prop_1, SlotFlag_OUTPUT, 1);

    auto* node_2 = graph->create_node();
    auto* prop_2 = node_2->add_prop<bool>("prop_2");
    auto* slot_2 = node_2->add_slot(prop_2, SlotFlag_INPUT, 1);

    // Act
    DirectedEdge& edge = *graph->connect_or_merge( *slot_1, *slot_2 );

    // Verify
    EXPECT_EQ(edge.tail->get_property(), prop_1 );
    EXPECT_EQ(edge.head->get_property(), prop_2 );
    EXPECT_EQ(graph->get_edge_registry().size(), 1);
 }

TEST_F(Graph_, disconnect)
{
    // Prepare
    Graph* graph = app.get_graph();
    auto node_1 = graph->create_node();
    auto prop_1 = node_1->add_prop<bool>("prop_1");
    auto slot_1 = node_1->add_slot(prop_1, SlotFlag_OUTPUT, 1);

    auto node_2 = graph->create_node();
    auto prop_2 = node_2->add_prop<bool>("prop_2");
    auto slot_2 = node_2->add_slot(prop_2, SlotFlag_INPUT, 1);

    EXPECT_EQ(graph->get_edge_registry().size(), 0);
    DirectedEdge& edge = *graph->connect_or_merge( *slot_1, *slot_2 );
    EXPECT_EQ(graph->get_edge_registry().size(), 1);

    // Act
    graph->disconnect(edge, ConnectFlag_ALLOW_SIDE_EFFECTS );

    // Check
    EXPECT_EQ(graph->get_edge_registry().size() , 0);
    EXPECT_EQ( node_1->adjacent_slot_count( SlotFlag_OUTPUT ), 0);
    EXPECT_EQ( node_2->adjacent_slot_count( SlotFlag_INPUT ) , 0);
}

TEST_F(Graph_, clear)
{
    Graph* graph = app.get_graph();
    EXPECT_TRUE( graph->get_node_registry().empty() );
    EXPECT_TRUE( graph->get_edge_registry().empty() );

    VariableNode*     variable  = graph->create_variable(type::get<int>(), "var", nullptr);
    FunctionDescriptor*         fct_type  = FunctionDescriptor::create<int(int, int)>("+");
    const IInvokable* invokable = app.get_language()->find_operator_fct_exact(fct_type);

    EXPECT_TRUE(invokable != nullptr);
    auto operator_node = graph->create_operator(fct_type);

    EXPECT_TRUE( graph->get_edge_registry().empty() );

    graph->connect(
            *operator_node->value_out(),
            *variable->value_in(),
            ConnectFlag_ALLOW_SIDE_EFFECTS);

    EXPECT_FALSE( graph->get_node_registry().empty() );
    EXPECT_FALSE( graph->get_edge_registry().empty() );

    // act
    graph->clear();

    // test
    EXPECT_TRUE( graph->get_node_registry().empty() );
    EXPECT_TRUE( graph->get_edge_registry().empty() );

    delete fct_type;
}


TEST_F(Graph_, create_and_delete_relations)
{
    // prepare
    Graph* graph = app.get_graph();
    auto& edges = graph->get_edge_registry();
    EXPECT_EQ(edges.size(), 0);
    auto node_1 = graph->create_scope();
    EXPECT_EQ(edges.size(), 0);
    auto node_2 = graph->create_node();

    // Act and test

    // is child of (and by reciprocity "is parent of")
    EXPECT_EQ(edges.size(), 0);
    EXPECT_EQ( Utils::get_adjacent_nodes( node_2, SlotFlag_TYPE_HIERARCHICAL ).size(), 0);
    DirectedEdge* edge_1 = graph->connect(
            *node_1->find_slot( SlotFlag_CHILD ),
            *node_2->find_slot( SlotFlag_PARENT ));
    EXPECT_EQ( Utils::get_adjacent_nodes( node_2, SlotFlag_TYPE_HIERARCHICAL ).size(), 1);
    EXPECT_EQ(edges.size(), 1);
    graph->disconnect(*edge_1);
    EXPECT_EQ( Utils::get_adjacent_nodes( node_2, SlotFlag_TYPE_HIERARCHICAL ).size(), 0);
}
