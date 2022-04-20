#include <gtest/gtest.h>

#include <nodable/core/Member.h>
#include <nodable/core/Node.h>
#include <nodable/core/GraphNode.h>
#include <nodable/core/InstructionNode.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/NodeFactory.h>
#include <nodable/core/Wire.h>
#include <nodable/core/languages/NodableLanguage.h>
#include <nodable/core/InvokableComponent.h>
#include <nodable/core/Scope.h>
#include <nodable/core/SignatureBuilder.h>

using namespace Nodable;

class graph_node_fixture: public ::testing::Test {
public:
    graph_node_fixture( )
    : factory(&language)
    , graph(&language, &factory, &autocompletion){}

    void SetUp( ) {
        // code here will execute just before the test ensues
    }

    void TearDown( ) {
        // code here will be called just after the test completes
        // ok to through exceptions from here if need be
    }

    ~graph_node_fixture( )  {
        // cleanup any pending stuff, but no exceptions allowed
    }

protected:
    const NodableLanguage language;
    NodeFactory           factory;
    bool                  autocompletion = false;
public:
    GraphNode             graph;
};

TEST_F( graph_node_fixture, connect)
{
    auto node1 = graph.create_node();
    node1->props()->add<bool>("output", Visibility::Default, Way_Default);

    auto node2  = graph.create_node();
    node2->props()->add<bool>("input", Visibility::Default, Way_Default);

    auto wire = graph.connect(
            node1->props()->get("output"),
            node2->props()->get("input"));

    EXPECT_EQ(wire->members.src , node1->props()->get("output"));
    EXPECT_EQ(wire->members.dst , node2->props()->get("input"));
    EXPECT_EQ(graph.get_wire_registry().size(), 1);
 }

TEST_F( graph_node_fixture, disconnect)
{
    Node*   a      = graph.create_node();
    Member* output = a->props()->add<bool>("output", Visibility::Default, Way_Default);

    Node*   b     = graph.create_node();
    Member* input = b->props()->add<bool>("input", Visibility::Default, Way_Default);

    EXPECT_EQ(graph.get_wire_registry().size()    , 0);
    EXPECT_EQ(graph.get_relation_registry().size(), 0);

    Wire* wire = graph.connect(output, input);

    EXPECT_EQ(graph.get_wire_registry().size()    , 1); // wire must be registered when connected
    EXPECT_EQ(graph.get_relation_registry().size(), 1); // relation must be registered when connected

    graph.disconnect(wire);

    EXPECT_EQ(graph.get_wires().size()  , 0); // wire must be unregistered when disconnected
    EXPECT_EQ(a->get_output_wire_count(), 0);
    EXPECT_EQ(b->get_input_wire_count() , 0);
}

TEST_F( graph_node_fixture, clear)
{
    InstructionNode* instructionNode = graph.create_instr();
    func_type*       sig             = SignatureBuilder<int(int, int)>::new_operator("+", &language);
    const iinvokable* operator_fct   = language.find_operator_fct_exact(sig);

    delete sig;

    EXPECT_TRUE(operator_fct != nullptr);
    Node* operatorNode = graph.create_function(operator_fct);
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


TEST_F( graph_node_fixture, create_and_delete_relations)
{
    // prepare
    Node* program = graph.create_root();
    EXPECT_EQ(graph.get_relation_registry().size(), 0);
    Node* n1 = graph.create_variable(type::get<double>(), "n1", program->get<Scope>());
    EXPECT_EQ(graph.get_relation_registry().size(), 0);
    Node* n2 = graph.create_variable(type::get<double>(), "n2", program->get<Scope>());

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
    EXPECT_EQ(n2->inputs().size(), 0);
    graph.connect({n1, EdgeType::IS_INPUT_OF, n2}, false);
    EXPECT_EQ(n2->inputs().size(), 1);
    EXPECT_EQ(graph.get_relation_registry().size(), 1);
    graph.disconnect({n1, EdgeType::IS_INPUT_OF, n2});
    EXPECT_EQ(n2->inputs().size(), 0);
    EXPECT_EQ(graph.get_relation_registry().size(), 0);
}
