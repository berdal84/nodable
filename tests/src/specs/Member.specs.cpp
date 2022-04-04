
#include <gtest/gtest.h>
#include <nodable/core/Member.h>

using namespace Nodable;
using namespace Nodable::R;

TEST(Member, Way_In)
{
    Member m(nullptr);
    m.set_allowed_connection(Way_In);

    EXPECT_FALSE(m.allows_connection(Way_Out));
    EXPECT_FALSE(m.allows_connection(Way_InOut));
    EXPECT_TRUE(m.allows_connection(Way_In));
    EXPECT_TRUE(m.allows_connection(Way_None));
}

TEST(Member, Way_Out)
{
    Member m(nullptr);
    m.set_allowed_connection(Way_Out);

    EXPECT_TRUE(m.allows_connection(Way_Out));
    EXPECT_FALSE(m.allows_connection(Way_InOut));
    EXPECT_FALSE(m.allows_connection(Way_In));
    EXPECT_TRUE(m.allows_connection(Way_None));
}

TEST(Member, Way_None)
{
    Member m(nullptr);
    m.set_allowed_connection(Way_Out);

    EXPECT_TRUE(m.allows_connection(Way_Out));
    EXPECT_FALSE(m.allows_connection(Way_InOut));
    EXPECT_FALSE(m.allows_connection(Way_In));
    EXPECT_TRUE(m.allows_connection(Way_None));
}

TEST(Member, Way_InOut)
{
    Member m(nullptr);
    m.set_allowed_connection(Way_InOut);

    EXPECT_TRUE(m.allows_connection(Way_Out));
    EXPECT_TRUE(m.allows_connection(Way_InOut));
    EXPECT_TRUE(m.allows_connection(Way_In));
    EXPECT_TRUE(m.allows_connection(Way_None));
}

TEST(Member, Type_Boolean)
{

    Member m(nullptr);

    m.set(true);
    EXPECT_TRUE((bool)m);
    EXPECT_EQ(m.get_meta_type()->get_type(), Type::bool_t);

    m.set(false);
    EXPECT_FALSE((bool)m);
    EXPECT_TRUE(m.get_data()->is_defined());

}

TEST(Member, Type_String)
{
    Member m;
    const std::string str = "Hello world !";
    m.set(str);

    EXPECT_EQ((std::string)m, str);
    EXPECT_TRUE(m.convert_to<bool>());
    EXPECT_EQ(m.get_meta_type()->get_type(), Type::string_t);
    EXPECT_TRUE(m.get_data()->is_defined());
}

TEST(Member, Type_Double)
{
    Member m(nullptr);
    m.set((double)50);

    EXPECT_EQ((double)m, (double)50);
    EXPECT_EQ(m.get_meta_type()->get_type(), Type::double_t);
    EXPECT_TRUE(m.get_data()->is_defined());
}

TEST(Member, Modify_by_reference_using_a_pointer)
{
    Member m(nullptr);
    m.set(50.0);

    EXPECT_EQ((double)m, 50.0);
    EXPECT_EQ(m.get_meta_type()->get_type(), Type::double_t);
    EXPECT_TRUE(m.get_data()->is_defined());

    double* ref = (double*)m;
    *ref = 100.0;

    EXPECT_EQ((double)m, 100.0);
}

TEST(Member, Modify_by_reference_using_a_reference)
{
    Member m1(nullptr, 50.0);
    Member m2(nullptr, 50.0);

    EXPECT_EQ((double)m1, 50.0);
    EXPECT_EQ(m1.get_meta_type()->get_type(), Type::double_t);
    EXPECT_TRUE(m1.get_data()->is_defined());

    auto add_right_to_left = [](double& a, double b) -> double { return  a = a + b; };
    add_right_to_left((double&)m1, (double)m2);

    EXPECT_EQ((double)m1, 100.0);
}