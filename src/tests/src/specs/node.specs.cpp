
#include <gtest/gtest.h>

#include "nodable/core/Node.h"
#include "nodable/core/Property.h"

using namespace ndbl;

TEST(Node, add_property_double)
{
    Node node;
    auto property = node.props.add<double>("val");
    property->set(100.0);

    EXPECT_EQ((double)*property, 100.0);
    EXPECT_EQ(property->convert_to<std::string>(), "100.0");
    EXPECT_TRUE((bool)property);
}