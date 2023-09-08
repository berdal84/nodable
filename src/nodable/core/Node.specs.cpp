#include <gtest/gtest.h>
#include "nodable/core/Node.h"

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