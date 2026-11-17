#include <gtest/gtest.h>

#include "reflection/Type_Descriptor.h"
#include "Graph.h"
#include "Node.h"
#include "Scope.h"

#include "test/fixtures/nodable_headless_test.h"

using namespace ndbl;
typedef ::testing::Nodable_Headless_Test Graph_;

Node* _graph_new_simple_node(Graph* graph)
{
    Node* node = memory_new<Node>();
    node_init(node);
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT, 1);
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_IN);
    
    graph_add(graph, node, graph_root_scope(graph) );

    return node;
}

TEST_F(Graph_, constructor)
{
    EXPECT_TRUE( graph_is_empty(app.graph));
    EXPECT_NE(graph_root(app.graph), nullptr );
}

TEST_F(Graph_, create_node)
{
    Node* node = _graph_new_simple_node(app.graph) ;
    EXPECT_EQ(node->scope, graph_root_scope(app.graph));
}

TEST_F(Graph_, connect)
{
    // Prepare
    Node*           node_1 = _graph_new_simple_node(app.graph);
    Node_Property*  prop_1 = node_add_prop<bool>(node_1, "prop_1");
    Node_Slot*      slot_1 = node_add_slot(node_1, prop_1, Node_Slot::Flag_OUTPUT, 1);

    Node*           node_2 = _graph_new_simple_node(app.graph);
    Node_Property*  prop_2 = node_add_prop<bool>(node_2, "prop_2");
    Node_Slot*      slot_2 = node_add_slot(node_2, prop_2, Node_Slot::Flag_INPUT, 1);

    // Act
    graph_connect_or_merge(slot_1, slot_2 );

    // Verify
    EXPECT_EQ(slot_1->property, prop_1 );
    EXPECT_EQ(slot_2->property, prop_2 );
 }

TEST_F(Graph_, disconnect)
{
    // Prepare
    Graph* graph = app.graph;
    Node*           node_1 = _graph_new_simple_node(app.graph);
    Node_Property*  prop_1 = node_add_prop<bool>(node_1, "prop_1");
    Node_Slot*      slot_1 = node_add_slot(node_1, prop_1, Node_Slot::Flag_OUTPUT, 1);

    Node*           node_2 = _graph_new_simple_node(app.graph);
    Node_Property*  prop_2 = node_add_prop<bool>(node_2, "prop_2");
    Node_Slot*      slot_2 = node_add_slot(node_2, prop_2, Node_Slot::Flag_INPUT, 1);

    graph_connect_or_merge(slot_1, slot_2 );

    // Act
    graph_disconnect(slot_1, slot_2, Graph_Flag_ALLOW_SIDE_EFFECTS );

    // Check
    EXPECT_EQ( node_adjacent_slot_count( node_2, Node_Slot::Flag_OUTPUT ), 0);
    EXPECT_EQ( node_adjacent_slot_count( node_2, Node_Slot::Flag_INPUT ) , 0);
}

TEST_F(Graph_, clear)
{
    Graph* graph = app.graph;
    EXPECT_TRUE( graph_is_empty(app.graph) );

    Type_Descriptor  f;
    type_init<int(int, int)>(&f, "+");

    Node* variable      = graph_new_node(graph, { .type = Node_Type_VARIABLE, .type_descriptor = type_get<int>(), .name = "var" });
    Node* operator_node = graph_new_node(graph, { .type = Node_Type_OPERATOR, .type_descriptor = &f });

    graph_connect(
            operator_node->value_out(),
            variable->value_in(),
            Graph_Flag_ALLOW_SIDE_EFFECTS);

    EXPECT_FALSE(graph_is_empty(app.graph));

    // act
    graph_reset(graph);

    // test
    EXPECT_TRUE( graph_is_empty(app.graph) );
    EXPECT_TRUE( graph->node_by_id.size == 1 && graph_root(graph) != nullptr );
}


TEST_F(Graph_, create_and_delete_relations)
{
    // prepare
    Node* node_1 = graph_new_node(app.graph, { .type = Node_Type_LITERAL, .type_descriptor = type_get<int>() });
    Node* node_2 = graph_new_node(app.graph, { .type = Node_Type_VARIABLE, .type_descriptor = type_get<int>(), .name = "a" });

    // Act and test

    // INPUT (and by reciprocity OUTPUT)
    EXPECT_EQ(node_get_adjacent_nodes(node_2, Node_Slot::Flag_TYPE_VALUE ).size, 0);
    graph_connect(node_1->value_out(), node_2->value_in());
    EXPECT_EQ(node_get_adjacent_nodes(node_2, Node_Slot::Flag_TYPE_VALUE ).size, 1);
    graph_disconnect(node_1->value_out(), node_2->value_in());
    EXPECT_EQ(node_get_adjacent_nodes(node_2, Node_Slot::Flag_TYPE_VALUE ).size, 0);
}

TEST_F(Graph_, erase_node_from_non_root_scope)
{
    // prepare
    Node* scope_node = graph_new_node(app.graph, { .type = Node_Type_SCOPE });
    graph_connect(graph_root(app.graph)->flow_enter(), scope_node->flow_in());
    Node* child = _graph_new_simple_node(app.graph);
    graph_change_node_scope(child, scope_node->internal_scope);

    EXPECT_EQ(child->scope, scope_node->internal_scope );

    graph_delete(app.graph, child);

    EXPECT_FALSE( graph_contains(app.graph, child ) );
    EXPECT_TRUE( scope_is_empty(scope_node->internal_scope) );
}


TEST_F(Graph_, erase_first_node_of_a_scope_with_another_child_after)
{
    // prepare
    Node* scope_node = graph_new_node(app.graph, { .type = Node_Type_SCOPE });
    Node* child1     = _graph_new_simple_node( app.graph );
    Node* child2     = _graph_new_simple_node( app.graph );

    graph_connect( scope_node->flow_enter(), child1->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );
    graph_connect( child1->flow_out(), child2->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );

    EXPECT_EQ(child1->scope, scope_node->internal_scope );
    EXPECT_EQ(child1->scope, child2->scope);

    graph_delete( app.graph, child1 );

    EXPECT_FALSE(graph_contains( app.graph, child1 ));
    EXPECT_TRUE(graph_contains( app.graph, child2 ));
    EXPECT_FALSE(scope_contains(scope_node->internal_scope, child1) );
    EXPECT_TRUE(scope_contains(scope_node->internal_scope, child2 ) );
}