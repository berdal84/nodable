#include <gtest/gtest.h>

// Hack to access private fields
#define private public

#include "ndbl/core/Node.h"

using namespace ndbl;

TEST(Node, constructor_destructor)
{
    {
        Node node;
        EXPECT_TRUE(node.type == Node_Type_NULL);
        EXPECT_FALSE(node.flags & Node_Flag_IS_INITIALIZED);
    }
}

TEST(Node, init_default_node)
{
    Node node;
    node_init(&node, Node_Type_NULL, "My Node");
    EXPECT_STREQ(node.name.c_str(), "My Node");
    EXPECT_TRUE(node.flags & Node_Flag_IS_INITIALIZED);
    node_deinit(&node);
}
