
#include "gtest/gtest.h"
#include <Core/Member.h>

using namespace Nodable;

TEST(Member, Way_In)
{
    auto m = std::make_unique<Member>(nullptr);
    m->setConnectorWay(Way_In);

    EXPECT_FALSE(m->allowsConnection(Way_Out));
    EXPECT_FALSE(m->allowsConnection(Way_InOut));
    EXPECT_TRUE(m->allowsConnection(Way_In));
    EXPECT_TRUE(m->allowsConnection(Way_None));
}

TEST(Member, Way_Out)
{
    auto m = std::make_unique<Member>(nullptr);
    m->setConnectorWay(Way_Out);

    EXPECT_TRUE(m->allowsConnection(Way_Out));
    EXPECT_FALSE(m->allowsConnection(Way_InOut));
    EXPECT_FALSE(m->allowsConnection(Way_In));
    EXPECT_TRUE(m->allowsConnection(Way_None));
}

TEST(Member, Way_None)
{
    auto m = std::make_unique<Member>(nullptr);
    m->setConnectorWay(Way_Out);

    EXPECT_TRUE(m->allowsConnection(Way_Out));
    EXPECT_FALSE(m->allowsConnection(Way_InOut));
    EXPECT_FALSE(m->allowsConnection(Way_In));
    EXPECT_TRUE(m->allowsConnection(Way_None));
}

TEST(Member, Way_InOut)
{
    auto m = std::make_unique<Member>(nullptr);
    m->setConnectorWay(Way_InOut);

    EXPECT_TRUE(m->allowsConnection(Way_Out));
    EXPECT_TRUE(m->allowsConnection(Way_InOut));
    EXPECT_TRUE(m->allowsConnection(Way_In));
    EXPECT_TRUE(m->allowsConnection(Way_None));
}

TEST(Member, Type_Boolean)
{

    auto m = std::make_unique<Member>(nullptr);

    m->set(true);
    EXPECT_TRUE((bool)*m);
    EXPECT_EQ(m->getType(), Type::Boolean);

    m->set(false);
    EXPECT_FALSE((bool)*m);
    EXPECT_TRUE(m->isDefined());

}

TEST(Member, Type_String)
{
    auto m = std::make_unique<Member>(nullptr);
    m->set("Hello world !");
    const std::string str = "Hello world !";

    EXPECT_EQ((std::string)*m, str);
    EXPECT_TRUE((bool)*m);
    EXPECT_EQ(m->getType(), Type::String);
    EXPECT_EQ((double)*m, str.length());
    EXPECT_TRUE(m->isDefined());
}

TEST(Member, Type_Double)
{
    auto m = std::make_unique<Member>(nullptr);
    m->set((double)50);

    EXPECT_EQ((double)*m, (double)50);
    EXPECT_EQ(m->getType(), Type::Double);
    EXPECT_TRUE(m->isDefined());
}