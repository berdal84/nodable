
#include "gtest/gtest.h"
#include <Member.h>

using namespace Nodable::core;

TEST(Member, Way_In)
{
    Member m;
    m.setConnectorWay(Way_In);

    EXPECT_FALSE(m.allowsConnection(Way_Out));
    EXPECT_FALSE(m.allowsConnection(Way_InOut));
    EXPECT_TRUE(m.allowsConnection(Way_In));
    EXPECT_TRUE(m.allowsConnection(Way_None));
}

TEST(Member, Way_Out)
{
    Member m;
    m.setConnectorWay(Way_Out);

    EXPECT_TRUE(m.allowsConnection(Way_Out));
    EXPECT_FALSE(m.allowsConnection(Way_InOut));
    EXPECT_FALSE(m.allowsConnection(Way_In));
    EXPECT_TRUE(m.allowsConnection(Way_None));
}

TEST(Member, Way_None)
{
    Member m;
    m.setConnectorWay(Way_Out);

    EXPECT_TRUE(m.allowsConnection(Way_Out));
    EXPECT_FALSE(m.allowsConnection(Way_InOut));
    EXPECT_FALSE(m.allowsConnection(Way_In));
    EXPECT_TRUE(m.allowsConnection(Way_None));
}

TEST(Member, Way_InOut)
{
    Member m;
    m.setConnectorWay(Way_InOut);

    EXPECT_TRUE(m.allowsConnection(Way_Out));
    EXPECT_TRUE(m.allowsConnection(Way_InOut));
    EXPECT_TRUE(m.allowsConnection(Way_In));
    EXPECT_TRUE(m.allowsConnection(Way_None));
}

TEST(Member, Type_Boolean)
{

    Member m;

    m.set(true);
    EXPECT_TRUE((bool)m);
    EXPECT_EQ(m.getType(), Type_Boolean);

    m.set(false);
    EXPECT_FALSE((bool)m);
    EXPECT_TRUE(m.isDefined());

}

TEST(Member, Type_String)
{
    Member m;
    m.set("Hello world !");
    const std::string str = "Hello world !";

    EXPECT_EQ((std::string)m, str);
    EXPECT_TRUE((bool)m);
    EXPECT_EQ(m.getType(), Type_String);
    EXPECT_EQ((double)m, str.length());
    EXPECT_TRUE(m.isDefined());
}

TEST(Member, Type_Double)
{
    Member m;
    m.set((double)50);

    EXPECT_EQ((double)m, (double)50);
    EXPECT_EQ(m.getType(), Type_Double);
    EXPECT_TRUE(m.isDefined());
}