#include <gtest/gtest.h>

#include "core/TDirectedEdge.h"
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
    auto node1 = graph.create_node();
    auto node2 = graph.create_node();

    node1->set_name("Node 1");
    node2->set_name("Node 2");

    auto id1 = node1->add_prop<bool>("output");
    auto id2 = node2->add_prop<bool>("input");

    DirectedEdge edge = graph.connect(node1->get_slot(id1, Way::Out), node2->get_slot(id2, Way::In));

    EXPECT_EQ(edge.tail.get_property(), node1->get_prop_at(id1) );
    EXPECT_EQ(edge.head.get_property(), node2->get_prop_at(id2) );
    EXPECT_EQ(graph.get_edge_registry().size(), 1);
 }

TEST_F(Graph_, disconnect)
{
    auto node_1 = graph.create_node();
    auto prop_1 = node_1->add_prop<bool>("prop_1");

    auto node_2 = graph.create_node();
    auto prop_2 = node_2->add_prop<bool>("prop_2");

    EXPECT_EQ(graph.get_edge_registry().size(), 0);

    DirectedEdge edge = graph.connect(node_1->get_slot(prop_1, Way::Out), node_2->get_slot(prop_2, Way::In));

    EXPECT_EQ(graph.get_edge_registry().size(), 1); // edge must be registered when connected

    graph.disconnect(edge, ConnectFlag::SIDE_EFFECTS_ON);

    EXPECT_EQ(graph.get_edge_registry().size()  , 0); // edge must be unregistered when disconnected
    EXPECT_EQ(node_1->outgoing_edge_count(), 0);
    EXPECT_EQ(node_2->incoming_edge_count() , 0);
}

TEST_F(Graph_, clear)
{
    auto             instructionNode = graph.create_instr();
    fw::func_type*   fct_type        = fw::func_type_builder<int(int, int)>::with_id("+");
    auto             operator_fct    = nodlang.find_operator_fct_exact(fct_type);

    EXPECT_TRUE(operator_fct.get() != nullptr);
    auto operatorNode = graph.create_operator(operator_fct.get());
    operatorNode->get_prop(LEFT_VALUE_PROPERTY)->set(2);
    operatorNode->get_prop(RIGHT_VALUE_PROPERTY)->set(2);

    graph.connect(
            operatorNode->get_slot(THIS_PROPERTY, Way::Out),
            instructionNode->get_slot(ROOT_PROPERTY, Way::In)
            );

    EXPECT_TRUE(graph.get_node_registry().size() != 0);
    EXPECT_TRUE(graph.get_edge_registry().size() != 0);

    // act
    graph.clear();

    // test
    EXPECT_EQ(graph.get_node_registry().size(), 0);
    EXPECT_EQ(graph.get_edge_registry().size(), 0);

    delete fct_type;
}


TEST_F(Graph_, create_and_delete_relations)
{
    // prepare
    auto double_type = fw::type::get<double>();
    ID<Scope> scope  = graph.create_root()->get_component<Scope>();
    auto& edges      = graph.get_edge_registry();
    EXPECT_EQ(edges.size(), 0);
    auto node_1 = graph.create_variable(double_type, "node_1", scope);
    EXPECT_EQ(edges.size(), 0);
    auto node_2 = graph.create_variable(double_type, "node_2", scope);

    // Act and test

    // is child of (and by reciprocity "is parent of")
    EXPECT_EQ(edges.size(), 0);
    EXPECT_EQ(node_2->filter_edges(Relation::NEXT_PREVIOUS).size(), 0);
    auto edge_1 = graph.connect(
            node_1->get_slot(THIS_PROPERTY, Way::Out),
            Relation::CHILD_PARENT,
            node_2->get_slot(THIS_PROPERTY, Way::In),
            ConnectFlag::SIDE_EFFECTS_OFF);
    EXPECT_EQ(node_2->filter_edges(Relation::CHILD_PARENT).size(), 1);
    EXPECT_EQ(edges.size(), 1);
    graph.disconnect(edge_1, ConnectFlag::SIDE_EFFECTS_OFF);
    EXPECT_EQ(node_2->filter_edges(Relation::CHILD_PARENT).size(), 0);

    // Is input of
    EXPECT_EQ(edges.size(), 0);
    EXPECT_EQ(node_2->filter_edges(Relation::READ_WRITE).size(), 0);
    auto edge_2 = graph.connect(
            node_1->get_value_slot(Way::Out),
            Relation::READ_WRITE,
            node_2->get_value_slot(Way::In),
            ConnectFlag::SIDE_EFFECTS_OFF);
    EXPECT_EQ(node_2->filter_edges(Relation::READ_WRITE).size(), 1);
    EXPECT_EQ(edges.size(), 1);
    graph.disconnect(edge_2, ConnectFlag::SIDE_EFFECTS_OFF);
    EXPECT_EQ(node_2->filter_edges(Relation::READ_WRITE).size(), 0);
    EXPECT_EQ(edges.size(), 0);
}
