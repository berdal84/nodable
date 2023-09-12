#include <gtest/gtest.h>

#include "core/DirectedEdge.h"
#include "fixtures/core.h"
#include "fw/core/reflection/func_type.h"
#include "nodable/core/Graph.h"
#include "nodable/core/InstructionNode.h"
#include "nodable/core/InvokableComponent.h"
#include "nodable/core/Node.h"
#include "nodable/core/NodeFactory.h"
#include "nodable/core/Property.h"
#include "nodable/core/Scope.h"
#include "nodable/core/VariableNode.h"
#include "nodable/core/language/Nodlang.h"

using namespace ndbl;
typedef ::testing::Core Graph_;

TEST_F(Graph_, connect)
{
    // Prepare
    auto node_1 = graph.create_node();
    auto prop_1 = node_1->add_prop<bool>("prop_1");
    auto slot_1 = node_1->add_slot( prop_1, SlotFlag_INPUT  );

    auto node_2 = graph.create_node();
    auto prop_2 = node_2->add_prop<bool>("prop_2");
    auto slot_2 = node_2->add_slot( prop_2, SlotFlag_OUTPUT );

    // Act
    DirectedEdge edge = graph.connect_or_digest( &node_1->get_slot( slot_1 ), &node_2->get_slot( slot_2 ) );

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
    auto slot_1 = node_1->add_slot( prop_1, SlotFlag_OUTPUT );

    auto node_2 = graph.create_node();
    auto prop_2 = node_2->add_prop<bool>("prop_2");
    auto slot_2 = node_2->add_slot( prop_2, SlotFlag_INPUT );

    EXPECT_EQ(graph.get_edge_registry().size(), 0);
    DirectedEdge edge = graph.connect_or_digest( &node_1->get_slot( slot_1 ), &node_2->get_slot( slot_2 ) );
    EXPECT_EQ(graph.get_edge_registry().size(), 1);

    // Act
    graph.disconnect(edge, SideEffects::ON );

    // Check
    EXPECT_EQ(graph.get_edge_registry().size() , 0);
    EXPECT_EQ(node_1->adjacent_count(SlotFlag_OUTPUT), 0);
    EXPECT_EQ(node_2->adjacent_count(SlotFlag_INPUT) , 0);
}

TEST_F(Graph_, clear)
{
    EXPECT_TRUE( graph.get_node_registry().empty() );
    EXPECT_TRUE( graph.get_edge_registry().empty() );

    auto             instructionNode = graph.create_instr();
    fw::func_type*   fct_type        = fw::func_type_builder<int(int, int)>::with_id("+");
    auto             operator_fct    = nodlang.find_operator_fct_exact(fct_type);

    EXPECT_TRUE(operator_fct.get() != nullptr);
    auto operator_node = graph.create_operator(operator_fct.get());
    operator_node->get_prop(LEFT_VALUE_PROPERTY)->set(2);
    operator_node->get_prop(RIGHT_VALUE_PROPERTY)->set(2);

    EXPECT_TRUE( graph.get_edge_registry().empty() );

    graph.connect(
            &operator_node->get_slot( THIS_PROPERTY, SlotFlag_OUTPUT ),
            &instructionNode->get_slot( ROOT_PROPERTY, SlotFlag_INPUT ),
            SideEffects::ON);

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
    auto double_type = fw::type::get<double>();
    PoolID<Scope> scope = graph.create_root()->get_component<Scope>();
    auto& edges = graph.get_edge_registry();
    EXPECT_EQ(edges.size(), 0);
    auto node_1 = graph.create_variable(double_type, "node_1", scope);
    EXPECT_EQ(edges.size(), 0);
    auto node_2 = graph.create_variable(double_type, "node_2", scope);

    // Act and test

    // is child of (and by reciprocity "is parent of")
    EXPECT_EQ(edges.size(), 0);
    EXPECT_EQ( node_2->filter_adjacent( SlotFlag_TYPE_HIERARCHICAL ).size(), 0);
    auto edge_1 = graph.connect(
            &node_1->get_slot( THIS_PROPERTY, SlotFlag_PARENT ),
            &node_2->get_slot( THIS_PROPERTY, SlotFlag_CHILD ),
            SideEffects::OFF );
    EXPECT_EQ( node_2->filter_adjacent( SlotFlag_TYPE_HIERARCHICAL ).size(), 1);
    EXPECT_EQ(edges.size(), 1);
    graph.disconnect(edge_1, SideEffects::OFF );
    EXPECT_EQ( node_2->filter_adjacent( SlotFlag_TYPE_HIERARCHICAL ).size(), 0);

    // Is input of
    EXPECT_EQ(edges.size(), 0);
    EXPECT_EQ( node_2->filter_adjacent( SlotFlag_TYPE_VALUE ).size(), 0);
    auto edge_2 = graph.connect(
            &node_1->get_value_slot( SlotFlag_OUTPUT ),
            &node_2->get_value_slot( SlotFlag_INPUT ),
            SideEffects::OFF );
    EXPECT_EQ( node_2->filter_adjacent( SlotFlag_TYPE_VALUE ).size(), 1);
    EXPECT_EQ(edges.size(), 1);
    graph.disconnect(edge_2, SideEffects::OFF );
    EXPECT_EQ( node_2->filter_adjacent( SlotFlag_TYPE_VALUE ).size(), 0);
    EXPECT_EQ(edges.size(), 0);
}
