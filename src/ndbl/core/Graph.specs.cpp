#include <gtest/gtest.h>

#include "tools/core/reflection/Type_Descriptor.h"

#include "Graph.h"
#include "Node.h"
#include "Scope.h"

#include "fixtures/core.h"

using namespace tools;
typedef ::testing::Core Graph_;

TEST_F(Graph_, constructor)
{
    EXPECT_TRUE( graph_is_empty(state.graph));
    EXPECT_NE(graph_root(state.graph), nullptr );
}

TEST_F(Graph_, create_node)
{
    Node* node = graph_create_node(state.graph) ;
    EXPECT_EQ(node->scope, graph_root_scope(state.graph));
}

TEST_F(Graph_, connect)
{
    // Prepare
    Graph* graph = state.graph;
    auto* node_1 = graph_create_node(state.graph, graph_root_scope(state.graph));
    auto* prop_1 = node_add_prop<bool>(node_1, "prop_1");
    auto* slot_1 = node_add_slot(node_1, prop_1, Node_Slot::Flag_OUTPUT, 1);

    auto* node_2 = graph_create_node(state.graph, graph_root_scope(state.graph));
    auto* prop_2 = node_add_prop<bool>(node_2, "prop_2");
    auto* slot_2 = node_add_slot(node_2, prop_2, Node_Slot::Flag_INPUT, 1);

    // Act
    graph_connect_or_merge(slot_1, slot_2 );

    // Verify
    EXPECT_EQ(slot_1->property, prop_1 );
    EXPECT_EQ(slot_2->property, prop_2 );
 }

TEST_F(Graph_, disconnect)
{
    // Prepare
    Graph* graph = state.graph;
    auto node_1 = graph_create_node(state.graph);
    auto prop_1 = node_add_prop<bool>(node_1, "prop_1");
    auto slot_1 = node_add_slot(node_1, prop_1, Node_Slot::Flag_OUTPUT, 1);

    auto node_2 = graph_create_node(state.graph);
    auto prop_2 = node_add_prop<bool>(node_2, "prop_2");
    auto slot_2 = node_add_slot(node_2, prop_2, Node_Slot::Flag_INPUT, 1);

    graph_connect_or_merge(slot_1, slot_2 );

    // Act
    graph_disconnect(slot_1, slot_2, Graph_Flag_ALLOW_SIDE_EFFECTS );

    // Check
    EXPECT_EQ( node_adjacent_slot_count( node_2, Node_Slot::Flag_OUTPUT ), 0);
    EXPECT_EQ( node_adjacent_slot_count( node_2, Node_Slot::Flag_INPUT ) , 0);
}

TEST_F(Graph_, clear)
{
    Graph* graph = state.graph;
    EXPECT_TRUE( graph_is_empty(state.graph) );

    Function_Descriptor  f;
    f.init<int(int, int)>("+");

    Node* variable      = graph_create_variable(graph, type::get<int>(), "var");
    Node* operator_node = graph_create_operator(graph, &f);


    graph_connect(
            operator_node->value_out(),
            variable->value_in(),
            Graph_Flag_ALLOW_SIDE_EFFECTS);

    EXPECT_FALSE(graph_is_empty(state.graph));

    // act
    graph_reset(graph);

    // test
    EXPECT_TRUE( graph_is_empty(state.graph) );
    EXPECT_TRUE( graph->nodes.size() == 1 && *graph->nodes.cbegin() == graph_root(graph) );
}


TEST_F(Graph_, create_and_delete_relations)
{
    // prepare
    Graph* graph = state.graph;
    auto node_1 = graph_create_literal<int>(graph);
    auto node_2 = graph_create_variable<int>( graph, "a" );

    // Act and test

    // INPUT (and by reciprocity OUTPUT)
    EXPECT_EQ(node_get_adjacent_nodes(node_2, Node_Slot::Flag_TYPE_VALUE ).size(), 0);
    graph_connect(node_1->value_out(), node_2->value_in());
    EXPECT_EQ(node_get_adjacent_nodes(node_2, Node_Slot::Flag_TYPE_VALUE ).size(), 1);
    graph_disconnect(node_1->value_out(), node_2->value_in());
    EXPECT_EQ(node_get_adjacent_nodes(node_2, Node_Slot::Flag_TYPE_VALUE ).size(), 0);
}

TEST_F(Graph_, erase_node_from_non_root_scope)
{
    // prepare
    Graph* graph = state.graph;
    Node* scope_node = graph_create_scope(graph);
    graph_connect(graph_root(graph)->flow_enter(), scope_node->flow_in());
    Node* child = graph_create_node(graph, scope_node->internal_scope);

    EXPECT_EQ(child->scope, scope_node->internal_scope );

    graph_find_and_destroy(graph, child );

    EXPECT_FALSE( graph_contains( graph, child ) );
    EXPECT_TRUE( scope_is_empty(scope_node->internal_scope) );
}


TEST_F(Graph_, erase_first_node_of_a_scope_with_another_child_after)
{
    // prepare
    Graph* graph = state.graph;

    Node* scope_node = graph_create_scope( graph );
    Node* child1     = graph_create_node( graph );
    Node* child2     = graph_create_node( graph );

    graph_connect( scope_node->flow_enter(), child1->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );
    graph_connect( child1->flow_out(), child2->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );

    EXPECT_EQ(child1->scope, scope_node->internal_scope );
    EXPECT_EQ(child1->scope, child2->scope);

    graph_find_and_destroy( graph, child1 );

    EXPECT_FALSE(graph_contains( graph, child1 ));
    EXPECT_TRUE(graph_contains( graph, child2 ));
    EXPECT_FALSE(scope_contains(scope_node->internal_scope, child1) );
    EXPECT_TRUE(scope_contains(scope_node->internal_scope, child2 ) );
}