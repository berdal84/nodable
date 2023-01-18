
#include <gtest/gtest.h>

#include <nodable/core/Node.h>
#include <nodable/core/Property.h>

using namespace ndbl;

class node_fixture: public ::testing::Test {
public:
    node_fixture( ){}

    void SetUp( ) {
        // code here will execute just before the test ensues
    }

    void TearDown( ) {
        // code here will be called just after the test completes
        // ok to through exceptions from here if need be
    }

    ~node_fixture( )  {
        // cleanup any pending stuff, but no exceptions allowed
    }

    Node node;
};

TEST_F(node_fixture, add_property_double)
{
    auto property = node.props()->add("val", Visibility::Default, type::get<double>(), Way_Default);
    property->set(100.0);

    EXPECT_EQ((double)*property, 100.0);
    EXPECT_EQ(property->convert_to<std::string>(), "100.0");
    EXPECT_TRUE((bool)property);
}