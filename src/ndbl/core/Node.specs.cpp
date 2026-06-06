#include <gtest/gtest.h>

// Hack to access private fields
#define private public

#include "ndbl/core/Node.h"

using namespace ndbl;

TEST(Node, constructor_destructor)
{
    {
        Node node;
        EXPECT_TRUE(node.type() == Node_Type_NULL);
        EXPECT_FALSE(node.is_initialized());
    }
}

TEST(Node, init_default_node)
{
    Node node;
    node.init("My Node");
    EXPECT_STREQ(node.name().c_str(), "My Node");
    EXPECT_TRUE(node.is_initialized());
    node.shutdown();
}
