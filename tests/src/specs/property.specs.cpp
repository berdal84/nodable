
#include <gtest/gtest.h>
#include <nodable/core/Property.h>

using namespace ndbl;

TEST(Property, Way_In)
{
    Property m(nullptr);
    m.set_allowed_connection(Way_In);

    EXPECT_FALSE(m.allows_connection(Way_Out));
    EXPECT_FALSE(m.allows_connection(Way_InOut));
    EXPECT_TRUE(m.allows_connection(Way_In));
    EXPECT_TRUE(m.allows_connection(Way_None));
}

TEST(Property, Way_Out)
{
    Property m(nullptr);
    m.set_allowed_connection(Way_Out);

    EXPECT_TRUE(m.allows_connection(Way_Out));
    EXPECT_FALSE(m.allows_connection(Way_InOut));
    EXPECT_FALSE(m.allows_connection(Way_In));
    EXPECT_TRUE(m.allows_connection(Way_None));
}

TEST(Property, Way_None)
{
    Property m(nullptr);
    m.set_allowed_connection(Way_Out);

    EXPECT_TRUE(m.allows_connection(Way_Out));
    EXPECT_FALSE(m.allows_connection(Way_InOut));
    EXPECT_FALSE(m.allows_connection(Way_In));
    EXPECT_TRUE(m.allows_connection(Way_None));
}

TEST(Property, Way_InOut)
{
    Property m(nullptr);
    m.set_allowed_connection(Way_InOut);

    EXPECT_TRUE(m.allows_connection(Way_Out));
    EXPECT_TRUE(m.allows_connection(Way_InOut));
    EXPECT_TRUE(m.allows_connection(Way_In));
    EXPECT_TRUE(m.allows_connection(Way_None));
}

TEST(Property, Type_Boolean)
{

    Property m(nullptr);

    m.set(true);
    EXPECT_TRUE((bool)m);
    EXPECT_EQ(m.get_type(), fw::type::get<bool>());

    m.set(false);
    EXPECT_FALSE((bool)m);
    EXPECT_TRUE(m.get_variant()->is_defined());

}

TEST(Property, Type_String)
{
    Property m;
    const std::string str = "Hello world !";
    m.set(str);

    EXPECT_EQ((std::string)m, str);
    EXPECT_TRUE(m.convert_to<bool>());
    EXPECT_EQ(m.get_type(), fw::type::get<std::string>());
    EXPECT_TRUE(m.get_variant()->is_defined());
}

TEST(Property, Type_Double)
{
    Property m(nullptr);
    m.set((double)50);

    EXPECT_EQ((double)m, (double)50);
    EXPECT_EQ(m.get_type(), fw::type::get<double>());
    EXPECT_TRUE(m.get_variant()->is_defined());
}

TEST(Property, Modify_by_reference_using_a_pointer)
{
    Property m(nullptr);
    m.set(50.0);

    EXPECT_EQ((double)m, 50.0);
    EXPECT_EQ(m.get_type(), fw::type::get<double>());
    EXPECT_TRUE(m.get_variant()->is_defined());

    double& ref = (double&)m;
    ref = 100.0;

    EXPECT_EQ((double)m, 100.0);
}

TEST(Property, Modify_by_reference_using_a_reference)
{
    Property m1(nullptr, 50.0);
    Property m2(nullptr, 50.0);

    EXPECT_EQ((double)m1, 50.0);
    EXPECT_EQ(m1.get_type(), fw::type::get<double>());
    EXPECT_TRUE(m1.get_variant()->is_defined());

    auto add_right_to_left = [](double& a, double b) -> double { return  a = a + b; };
    add_right_to_left((double&)m1, (double)m2);

    EXPECT_EQ((double)m1, 100.0);
}