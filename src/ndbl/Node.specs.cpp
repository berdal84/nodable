#include <gtest/gtest.h>

#include "Node.h"
#include "reflection/index.h"
#include "Log.h"
#include "test/fixtures/basic_test.h"

using namespace ndbl;

typedef ::testing::Basic_Test Node_;

TEST_F(Node_, ctor_and_dtor )
{
    {
        Node node;
        EXPECT_TRUE(node.kind == Node_Kind_NULL);
        EXPECT_FALSE(node.flags & Node_Flag_IS_INITIALIZED);
    }
}

TEST_F(Node_, init_a_Node_Kind_NULL)
{
    Node node;
    node_init(&node, { .name = string("My Node") });
    EXPECT_STREQ(c_str(node.name), "My Node");
    EXPECT_TRUE(node.flags & Node_Flag_IS_INITIALIZED);
    node_deinit(&node);
}

TEST_F(Node_, copy)
{
    Node node;
    node_init(&node, {.name = string("My Node") });
    
    Node copy = node;
    EXPECT_STREQ(c_str(copy.name), "My Node");
    EXPECT_TRUE(copy.flags & Node_Flag_IS_INITIALIZED);
}
