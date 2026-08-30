#include <gtest/gtest.h>

// Hack to access private fields
#define private public

#include "ndbl/core/Node.h"

using namespace ndbl;

TEST(Node, ctor_and_dtor )
{
    {
        Node node;
        EXPECT_TRUE(node.type == Node_Type_NULL);
        EXPECT_FALSE(node.flags & Node_Flag_IS_INITIALIZED);
    }
}

TEST(Node, init_a_Node_Type_NULL)
{
    Node node;
    node_init(&node, Node_Type_NULL, "My Node");
    EXPECT_STREQ(node.name.c_str(), "My Node");
    EXPECT_TRUE(node.flags & Node_Flag_IS_INITIALIZED);
    node_deinit(&node);
}

TEST(Node, copy)
{
    Node node;
    node_init(&node, Node_Type_NULL, "My Node");
    
    Node copy = node;
    EXPECT_STREQ(copy.name.c_str(), "My Node");
    EXPECT_TRUE(copy.flags & Node_Flag_IS_INITIALIZED);
}
