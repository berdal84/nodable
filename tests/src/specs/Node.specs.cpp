
#include <gtest/gtest.h>

#include <nodable/Member.h>
#include <nodable/Node.h>

using namespace Nodable;
using namespace Nodable::R;

TEST(Node, Add_member_Double)
{
    Node node;
    auto props = node.props();
    props->add("val", Visibility::Default, Type::Double, Way_Default);
    props->get("val")->set(100.0);

    Member* val = props->get("val");

    EXPECT_EQ((double)*val, 100.0);
    EXPECT_EQ(val->convert_to<std::string>(), "100");
    EXPECT_TRUE((bool)val);
}