#include "ndbl/core/Scope.h"
#include "fixtures/core.h"
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

TEST_F(Node_, find_parent)
{
    auto parent = app.graph.create_scope();
    auto child  = app.graph.create_node();

    app.graph.connect(
        *parent->find_slot( SlotFlag_CHILD ),
        *child->find_slot( SlotFlag_PARENT )
    );

    EXPECT_TRUE( child->find_parent() );
    EXPECT_EQ( child->find_parent(), parent );
}