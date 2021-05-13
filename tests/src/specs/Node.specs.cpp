
#include "gtest/gtest.h"
#include <Member.h>
#include <Node.h>

using namespace Nodable;

TEST(Node, Add_member_Double)
{
    Node node;
    auto props = node.getProps();
    props->add("val", Visibility::Default, Type_Double, Way_Default);
    props->get("val")->set(100.0);

    auto val = props->get("val");

    EXPECT_EQ((double)*val, 100.0);
    EXPECT_EQ((std::string)*val, "100");
    EXPECT_TRUE((bool)*val);
}