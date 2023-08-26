
#include <gtest/gtest.h>

#include "gui/NodeView.h"
#include "nodable/core/Node.h"
#include "nodable/core/Property.h"

using namespace ndbl;

TEST(Node, add_property_double)
{
    Node node;
    auto property = node.add_prop<double>("val");
    property->set(100.0);

    EXPECT_EQ((double)*property->value(), 100.0);
    EXPECT_EQ(property->to<std::string>(), "100.0");
    EXPECT_TRUE(property->to<bool>());
}