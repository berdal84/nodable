#include <gtest/gtest.h>

#include "tools/core/reflection/Type_Descriptor.h"

#include "Graph.h"
#include "Node.h"
#include "Scope.h"
#include "Node_Slot_Link.h"

#include "fixtures/core.h"

using namespace ndbl;
using namespace tools;
typedef ::testing::Core Graph_;

TEST_F(Graph_, constructor)
{
    EXPECT_TRUE(app.graph()->is_empty());
    EXPECT_NE(app.graph()->root_node(), nullptr );
}

TEST_F(Graph_, create_node)
{
    Node* node = app.graph()->create_node();
    EXPECT_EQ(node->scope, app.graph()->root_scope());
}

TEST_F(Graph_, connect)
{
    // Prepare
    Graph* graph = app.graph();
    auto* node_1 = graph->create_node();
    auto* prop_1 = node_add_prop<bool>(node_1, "prop_1");
    auto* slot_1 = node_add_slot(node_1, prop_1, Node_Slot_Flag_OUTPUT, 1);

    auto* node_2 = graph->create_node();
    auto* prop_2 = node_add_prop<bool>(node_2, "prop_2");
    auto* slot_2 = node_add_slot(node_2, prop_2, Node_Slot_Flag_INPUT, 1);

    // Act
    Node_Slot_Link edge = graph->connect_or_merge(slot_1, slot_2 );

    // Verify
    EXPECT_EQ(edge.tail->property, prop_1 );
    EXPECT_EQ(edge.head->property, prop_2 );
    EXPECT_EQ(graph->edges().size(), 1);
 }

TEST_F(Graph_, disconnect)
{
    // Prepare
    Graph* graph = app.graph();
    auto node_1 = graph->create_node();
    auto prop_1 = node_add_prop<bool>(node_1, "prop_1");
    auto slot_1 = node_add_slot(node_1, prop_1, Node_Slot_Flag_OUTPUT, 1);

    auto node_2 = graph->create_node();
    auto prop_2 = node_add_prop<bool>(node_2, "prop_2");
    auto slot_2 = node_add_slot(node_2, prop_2, Node_Slot_Flag_INPUT, 1);

    EXPECT_EQ(graph->edges().size(), 0);
    Node_Slot_Link edge = graph->connect_or_merge(slot_1, slot_2 );
    EXPECT_EQ(graph->edges().size(), 1);

    // Act
    graph->disconnect(edge, Graph_Flag_ALLOW_SIDE_EFFECTS );

    // Check
    EXPECT_EQ(graph->edges().size() , 0);
    EXPECT_EQ( node_adjacent_slot_count( node_2, Node_Slot_Flag_OUTPUT ), 0);
    EXPECT_EQ( node_adjacent_slot_count( node_2, Node_Slot_Flag_INPUT ) , 0);
}

TEST_F(Graph_, clear)
{
    Graph* graph = app.graph();
    EXPECT_TRUE( graph->is_empty() );
    EXPECT_TRUE(graph->edges().empty() );

    Function_Descriptor  f;
    f.init<int(int, int)>("+");

    Node* variable  = graph->create_variable(type::get<int>(), "var");
    auto operator_node = graph->create_operator(f);

    EXPECT_TRUE(graph->edges().empty() );

    graph->connect(
            operator_node->value_out(),
            variable->value_in(),
            Graph_Flag_ALLOW_SIDE_EFFECTS);

    EXPECT_FALSE( graph->is_empty() );
    EXPECT_FALSE(graph->edges().empty() );

    // act
    graph->reset();

    // test
    EXPECT_TRUE( graph->is_empty() );
    EXPECT_TRUE( graph->nodes().size() == 1 && *graph->nodes().cbegin() == graph->root_node() );
    EXPECT_TRUE(graph->edges().empty() );
}


TEST_F(Graph_, create_and_delete_relations)
{
    // prepare
    Graph* graph = app.graph();
    auto& edges = graph->edges();
    EXPECT_EQ(edges.size(), 0);
    auto node_1 = graph->create_literal<int>();
    EXPECT_EQ(edges.size(), 0);
    auto node_2 = graph->create_variable( type::get<int>(), "a" );

    // Act and test

    // INPUT (and by reciprocity OUTPUT)
    EXPECT_EQ(edges.size(), 0);
    EXPECT_EQ(node_get_adjacent_nodes(node_2, Node_Slot_Flag_TYPE_VALUE ).size(), 0);
    Node_Slot_Link edge_1 = graph->connect(node_1->value_out(), node_2->value_in());
    EXPECT_EQ(node_get_adjacent_nodes(node_2, Node_Slot_Flag_TYPE_VALUE ).size(), 1);
    EXPECT_EQ(edges.size(), 1);
    graph->disconnect(edge_1);
    EXPECT_EQ(node_get_adjacent_nodes(node_2, Node_Slot_Flag_TYPE_VALUE ).size(), 0);
}

TEST_F(Graph_, erase_node_from_non_root_scope)
{
    // prepare
    Graph* graph = app.graph();
    Node* scope_node = graph->create_scope(graph->root_scope() );
    graph->connect(graph->root_node()->flow_enter(), scope_node->flow_in());
    Node* child = graph->create_node(scope_node->internal_scope);

    EXPECT_EQ(child->scope, scope_node->internal_scope );

    graph->find_and_destroy( child );

    EXPECT_FALSE( graph->contains( child ) );
    EXPECT_TRUE(scope_node->internal_scope->empty() );
}


TEST_F(Graph_, erase_first_node_of_a_scope_with_another_child_after)
{
    // prepare
    Graph* graph = app.graph();

    Node* scope_node = graph->create_scope( graph->root_scope() );
    Node* child1 = graph->create_node();
    Node* child2 = graph->create_node();

    graph->connect( scope_node->flow_enter(), child1->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );
    graph->connect( child1->flow_out(), child2->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );

    EXPECT_EQ(child1->scope, scope_node->internal_scope );
    EXPECT_EQ(child1->scope, child2->scope);

    graph->find_and_destroy( child1 );

    EXPECT_FALSE(graph->contains( child1 ));
    EXPECT_TRUE(graph->contains( child2 ));
    EXPECT_FALSE(scope_node->internal_scope->contains(child1) );
    EXPECT_TRUE(scope_node->internal_scope->contains(child2 ) );
}