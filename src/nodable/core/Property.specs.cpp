#include <gtest/gtest.h>
#include "fw/core/Pool.h"
#include "Property.h"

using namespace ndbl;
using namespace fw;

TEST(Property, Type_Boolean)
{
    Property property;

    property.set(true);
    EXPECT_TRUE((bool)*property);
    EXPECT_EQ(property.get_type(), type::get<bool>());

    property.set(false);
    EXPECT_FALSE((bool)*property);
    EXPECT_TRUE(property.value()->is_defined());

}

TEST(Property, Type_String)
{
    Property property;
    const std::string str = "Hello world !";
    property.set(str);

    EXPECT_EQ((std::string)*property, str);
    EXPECT_TRUE(property.to<bool>());
    EXPECT_EQ(property.get_type(), type::get<std::string>());
    EXPECT_TRUE(property->is_defined());
}

TEST(Property, Type_Double)
{
    Property property;
    property.set((double)50);

    EXPECT_EQ((double)50, (double)*property);
    EXPECT_EQ(property.get_type(), type::get<double>());
    EXPECT_TRUE(property->is_defined());
}

TEST(Property, Modify_by_reference_using_a_pointer)
{
    Property property;
    property.set(50.0);

    EXPECT_EQ((double)*property, 50.0);
    EXPECT_EQ(property.get_type(), type::get<double>());
    EXPECT_TRUE(property->is_defined());

    double& ref = (double&)*property;
    ref = 100.0;

    EXPECT_EQ((double)*property, 100.0);
}

TEST(Property, Modify_by_reference_using_a_reference)
{
    Property property_1(50.0);
    Property property_2(50.0);

    EXPECT_EQ((double)*property_1, 50.0);
    EXPECT_EQ(property_1.get_type(), type::get<double>());
    EXPECT_TRUE(property_1->is_defined());

    auto add_right_to_left = [](double& a, double b) -> double { return  a = a + b; };
    add_right_to_left((double&)*property_1, (double)*property_2);

    EXPECT_EQ((double)*property_1, 100.0);
}