
#include <gtest/gtest.h>

#include <nodable/core/Member.h>
#include <nodable/core/Node.h>

using namespace Nodable;

TEST(Node, Add_member_Double)
{
    Node node;
    node.props()->add("val", Visibility::Default, type::get<double>(), Way_Default);
    node.props()->get("val")->set(100.0);

    Member* val = node.props()->get("val");

    EXPECT_EQ((double)*val, 100.0);
    EXPECT_EQ(val->convert_to<std::string>(), "100.0");
    EXPECT_TRUE((bool)val);
}