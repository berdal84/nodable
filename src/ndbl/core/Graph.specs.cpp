#include <gtest/gtest.h>

#include "tools/core/reflection/func_type.h"

#include "Graph.h"
#include "InvokableComponent.h"
#include "Node.h"
#include "Scope.h"
#include "DirectedEdge.h"

#include "fixtures/core.h"

using namespace ndbl;
using namespace tools;
typedef ::testing::Core Graph_;

TEST_F(Graph_, connect)
{
    // Prepare
    auto node_1 = graph.create_node();
    auto prop_1 = node_1->add_prop<bool>("prop_1");
    auto slot_1 = node_1->add_slot( SlotFlag_OUTPUT, 1, prop_1  );

    auto node_2 = graph.create_node();
    auto prop_2 = node_2->add_prop<bool>("prop_2");
    auto slot_2 = node_2->add_slot( SlotFlag_INPUT, 1, prop_2 );

    // Act
    DirectedEdge& edge = *graph.connect_or_merge( node_1->get_slot_at( slot_1 ), node_2->get_slot_at( slot_2 ) );

    // Verify
    EXPECT_EQ(edge.tail->get_property(), node_1->get_prop_at( prop_1 ) );
    EXPECT_EQ(edge.head->get_property(), node_2->get_prop_at( prop_2 ) );
    EXPECT_EQ(graph.get_edge_registry().size(), 1);
 }

TEST_F(Graph_, disconnect)
{
    // Prepare
    auto node_1 = graph.create_node();
    auto prop_1 = node_1->add_prop<bool>("prop_1");
    auto slot_1 = node_1->add_slot( SlotFlag_OUTPUT, 1, prop_1 );

    auto node_2 = graph.create_node();
    auto prop_2 = node_2->add_prop<bool>("prop_2");
    auto slot_2 = node_2->add_slot( SlotFlag_INPUT, 1, prop_2 );

    EXPECT_EQ(graph.get_edge_registry().size(), 0);
    DirectedEdge& edge = *graph.connect_or_merge( node_1->get_slot_at( slot_1 ), node_2->get_slot_at( slot_2 ) );
    EXPECT_EQ(graph.get_edge_registry().size(), 1);

    // Act
    graph.disconnect(edge, ConnectFlag_ALLOW_SIDE_EFFECTS );

    // Check
    EXPECT_EQ(graph.get_edge_registry().size() , 0);
    EXPECT_EQ( node_1->adjacent_slot_count( SlotFlag_OUTPUT ), 0);
    EXPECT_EQ( node_2->adjacent_slot_count( SlotFlag_INPUT ) , 0);
}

TEST_F(Graph_, clear)
{
    EXPECT_TRUE( graph.get_node_registry().empty() );
    EXPECT_TRUE( graph.get_edge_registry().empty() );

    auto             variable        = graph.create_variable( type::get<int>(), "var", PoolID<Scope>::null);
    func_type*   fct_type        = func_type_builder<int(int, int)>::with_id("+");
    auto             operator_fct    = nodlang.find_operator_fct_exact(fct_type);

    EXPECT_TRUE(operator_fct.get() != nullptr);
    auto operator_node = graph.create_operator(operator_fct.get());
    operator_node->get_prop(LEFT_VALUE_PROPERTY)->set(2);
    operator_node->get_prop(RIGHT_VALUE_PROPERTY)->set(2);

    EXPECT_TRUE( graph.get_edge_registry().empty() );

    graph.connect(
            *operator_node->find_slot_by_property_name( VALUE_PROPERTY, SlotFlag_OUTPUT ),
            variable->input_slot(),
            ConnectFlag_ALLOW_SIDE_EFFECTS);

    EXPECT_FALSE( graph.get_node_registry().empty() );
    EXPECT_FALSE( graph.get_edge_registry().empty() );

    // act
    graph.clear();

    // test
    EXPECT_TRUE( graph.get_node_registry().empty() );
    EXPECT_TRUE( graph.get_edge_registry().empty() );

    delete fct_type;
}


TEST_F(Graph_, create_and_delete_relations)
{
    // prepare
    auto& edges = graph.get_edge_registry();
    EXPECT_EQ(edges.size(), 0);
    auto node_1 = graph.create_scope();
    EXPECT_EQ(edges.size(), 0);
    auto node_2 = graph.create_node();

    // Act and test

    // is child of (and by reciprocity "is parent of")
    EXPECT_EQ(edges.size(), 0);
    EXPECT_EQ( node_2->filter_adjacent( SlotFlag_TYPE_HIERARCHICAL ).size(), 0);
    DirectedEdge* edge_1 = graph.connect(
            *node_1->find_slot( SlotFlag_CHILD ),
            *node_2->find_slot( SlotFlag_PARENT ));
    EXPECT_EQ( node_2->filter_adjacent( SlotFlag_TYPE_HIERARCHICAL ).size(), 1);
    EXPECT_EQ(edges.size(), 1);
    graph.disconnect(*edge_1);
    EXPECT_EQ( node_2->filter_adjacent( SlotFlag_TYPE_HIERARCHICAL ).size(), 0);
}
