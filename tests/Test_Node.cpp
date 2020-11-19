
#include "gtest/gtest.h"
#include <Core/Member.h>
#include <Node.h>

using namespace Nodable;

TEST(Node, Add_member_Double)
{
    std::unique_ptr<Node> node(new Node);
    node->add("val");
    node->set("val", double(100));

    auto val = node->get("val");

    EXPECT_EQ((double)*val, double(100));
    EXPECT_EQ((std::string)*val, std::to_string(100));
    EXPECT_TRUE((bool)*val);
}