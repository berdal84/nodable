#include <gtest/gtest.h>
#include <nodable/core/reflection/R.h>
#include <nodable/core/Node.h>

using namespace Nodable;
using namespace Nodable::R;

TEST(Reflect, is_convertible__type_to_ptr)
{
    EXPECT_TRUE(Meta_t::is_implicitly_convertible(R::meta<bool>(), R::meta<bool *>()) );
    EXPECT_TRUE(Meta_t::is_implicitly_convertible(R::meta<double>(), R::meta<double *>()) );
    EXPECT_TRUE(Meta_t::is_implicitly_convertible(R::meta<std::string>(), R::meta<std::string *>())  );
}

TEST(Reflect, is_convertible__ptr_to_type)
{
    EXPECT_TRUE(Meta_t::is_implicitly_convertible(R::meta<bool *>(), R::meta<bool>()) );
    EXPECT_TRUE(Meta_t::is_implicitly_convertible(R::meta<double *>(), R::meta<double>()) );
    EXPECT_TRUE(Meta_t::is_implicitly_convertible(R::meta<std::string *>(), R::meta<std::string>()) );
    EXPECT_TRUE(Meta_t::is_implicitly_convertible(R::Meta_t::s_any, R::Meta_t::s_any));
}

TEST(Reflect, is_convertible__compatible_types)
{
    EXPECT_TRUE(Meta_t::is_implicitly_convertible(R::Meta_t::s_any, R::meta<double>()) );
    EXPECT_TRUE(Meta_t::is_implicitly_convertible(R::Meta_t::s_any, R::meta<std::string>()) );
    EXPECT_TRUE(Meta_t::is_implicitly_convertible(R::Meta_t::s_any, R::meta<bool>()) );
    EXPECT_TRUE(Meta_t::is_implicitly_convertible(R::Meta_t::s_any, R::meta<void>()) );
    EXPECT_TRUE(Meta_t::is_implicitly_convertible(R::Meta_t::s_any, R::Meta_t::s_any) );

    EXPECT_TRUE(Meta_t::is_implicitly_convertible(R::meta<void>(), R::Meta_t::s_any) );
    EXPECT_TRUE(Meta_t::is_implicitly_convertible(R::meta<double>(), R::Meta_t::s_any) );
    EXPECT_TRUE(Meta_t::is_implicitly_convertible(R::meta<std::string>(), R::Meta_t::s_any) );
    EXPECT_TRUE(Meta_t::is_implicitly_convertible(R::meta<bool>(), R::Meta_t::s_any) );
    EXPECT_TRUE(Meta_t::is_implicitly_convertible(R::Meta_t::s_any, R::Meta_t::s_any) );
}

TEST(Reflect, is_convertible__incompatible_types)
{
    EXPECT_FALSE(Meta_t::is_implicitly_convertible(R::meta<bool>(), R::meta<double>()) );
    EXPECT_FALSE(Meta_t::is_implicitly_convertible(R::meta<double>(), R::meta<bool>()) );

    EXPECT_FALSE(Meta_t::is_implicitly_convertible(R::meta<bool>(), R::meta<std::string>()) );
    EXPECT_FALSE(Meta_t::is_implicitly_convertible(R::meta<std::string>(), R::meta<bool>()) );

    EXPECT_FALSE(Meta_t::is_implicitly_convertible(R::meta<double>(), R::meta<std::string>()) );
    EXPECT_FALSE(Meta_t::is_implicitly_convertible(R::meta<std::string>(), R::meta<double>()) );
}

TEST(Reflect, is_ptr)
{
    EXPECT_FALSE(Meta_t::is_ptr(R::meta<bool &>()) );
    EXPECT_FALSE(Meta_t::is_ptr(R::meta<bool>()) );
    EXPECT_TRUE(Meta_t::is_ptr(R::meta<bool *>()) );
}

TEST(Reflect, is_ref)
{
    EXPECT_TRUE(Meta_t::is_ref(R::meta<bool &>()) );
    EXPECT_FALSE(Meta_t::is_ref(R::meta<bool>()) );
    EXPECT_FALSE(Meta_t::is_ref(R::meta<bool *>()) );
}

TEST(Reflect, node_as_pointer)
{
    // prepare
    Node* node_ptr = nullptr;
    auto member = std::make_unique<Member>(nullptr);

    // act
    member->set(node_ptr);

    // check
    EXPECT_TRUE(member->get_meta_type()->is_exactly(R::meta<Node *>()) );
    EXPECT_EQ(node_ptr, (Node*)*member );
    EXPECT_TRUE(Meta_t::is_ptr(member->get_meta_type()) );
}