#include "nodable/core/Node.h"
#include "Scope.h"
#include "core/fixtures/core.h"
#include <gtest/gtest.h>

using namespace ndbl;

TEST(Node, add_property_double)
{
    Node node;
    auto property_id = node.add_prop<double>("val");
    Property* property = node.get_prop_at(property_id);
    property->set(100.0);

    EXPECT_EQ((double)*property->value(), 100.0);
    EXPECT_EQ(property->to<std::string>(), "100.0");
    EXPECT_TRUE(property->to<bool>());
}

typedef ::testing::Core Node_;

TEST_F(Node_, get_parent)
{
    auto child  = graph.create_node();
    auto parent = graph.create_scope();

    graph.connect(
            parent->find_slot( THIS_PROPERTY, SlotFlag_CHILD ),
            child->find_slot( THIS_PROPERTY, SlotFlag_PARENT )
            );

    EXPECT_TRUE( child->get_parent() );
    EXPECT_EQ( child->get_parent(), parent );
}