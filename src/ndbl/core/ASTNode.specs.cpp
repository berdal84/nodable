#include <gtest/gtest.h>

// Hack to access private fields
#define private public

#include "ndbl/core/ASTNode.h"

using namespace ndbl;

TEST(ASTNode, constructor_destructor)
{
    {
        ASTNode node;
        EXPECT_TRUE(node.type() == ASTNodeType_NULL);
        EXPECT_FALSE(node.is_initialized());
    }
}

TEST(ASTNode, init_default_node)
{
    ASTNode node;
    node.init("My Node");
    EXPECT_STREQ(node.name().c_str(), "My Node");
    EXPECT_TRUE(node.is_initialized());
    node.shutdown();
}
