#include <gtest/gtest.h>
#include <nodable/R.h>
#include <nodable/Node.h>

using namespace Nodable;
using namespace Nodable::R;

TEST(Reflect, is_convertible__type_to_ptr)
{
    EXPECT_TRUE(MetaType::is_convertible(R::get_meta_type<bool>(), R::get_meta_type<bool *>() ) );
    EXPECT_TRUE(MetaType::is_convertible(R::get_meta_type<double>(), R::get_meta_type<double *>()) );
    EXPECT_TRUE(MetaType::is_convertible(R::get_meta_type<std::string>(), R::get_meta_type<std::string *>())  );
}

TEST(Reflect, is_convertible__ptr_to_type)
{
    EXPECT_TRUE(MetaType::is_convertible(R::get_meta_type<bool *>(), R::get_meta_type<bool>()) );
    EXPECT_TRUE(MetaType::is_convertible(R::get_meta_type<double *>(), R::get_meta_type<double>()) );
    EXPECT_TRUE(MetaType::is_convertible(R::get_meta_type<std::string *>(), R::get_meta_type<std::string>()) );
    EXPECT_TRUE(MetaType::is_convertible(R::MetaType::s_unknown, R::MetaType::s_unknown ));
}

TEST(Reflect, is_convertible__compatible_types)
{
    EXPECT_TRUE(MetaType::is_convertible(R::MetaType::s_unknown, R::get_meta_type<double>()) );
    EXPECT_TRUE(MetaType::is_convertible(R::MetaType::s_unknown, R::get_meta_type<std::string>()) );
    EXPECT_TRUE(MetaType::is_convertible(R::MetaType::s_unknown, R::get_meta_type<bool>()) );
    EXPECT_TRUE(MetaType::is_convertible(R::MetaType::s_unknown, R::get_meta_type<void>()) );
    EXPECT_TRUE(MetaType::is_convertible(R::MetaType::s_unknown, R::MetaType::s_unknown) );

    EXPECT_TRUE(MetaType::is_convertible(R::get_meta_type<void>(), R::MetaType::s_unknown) );
    EXPECT_TRUE(MetaType::is_convertible(R::get_meta_type<double>(), R::MetaType::s_unknown) );
    EXPECT_TRUE(MetaType::is_convertible(R::get_meta_type<std::string>(), R::MetaType::s_unknown) );
    EXPECT_TRUE(MetaType::is_convertible(R::get_meta_type<bool>(), R::MetaType::s_unknown) );
    EXPECT_TRUE(MetaType::is_convertible(R::MetaType::s_unknown, R::MetaType::s_unknown) );
}

TEST(Reflect, is_convertible__incompatible_types)
{
    EXPECT_FALSE(MetaType::is_convertible(R::get_meta_type<bool>(), R::get_meta_type<double>()) );
    EXPECT_FALSE(MetaType::is_convertible(R::get_meta_type<double>(), R::get_meta_type<bool>()) );

    EXPECT_FALSE(MetaType::is_convertible(R::get_meta_type<bool>(), R::get_meta_type<std::string>()) );
    EXPECT_FALSE(MetaType::is_convertible(R::get_meta_type<std::string>(), R::get_meta_type<bool>()) );

    EXPECT_FALSE(MetaType::is_convertible(R::get_meta_type<double>(), R::get_meta_type<std::string>()) );
    EXPECT_FALSE(MetaType::is_convertible(R::get_meta_type<std::string>(), R::get_meta_type<double>()) );
}

TEST(Reflect, is_ptr)
{
    EXPECT_FALSE(MetaType::is_ptr(R::get_meta_type<bool &>()) );
    EXPECT_FALSE(MetaType::is_ptr(R::get_meta_type<bool>()) );
    EXPECT_TRUE(MetaType::is_ptr(R::get_meta_type<bool *>()) );
}

TEST(Reflect, is_ref)
{
    EXPECT_TRUE(MetaType::is_ref(R::get_meta_type<bool &>()) );
    EXPECT_FALSE(MetaType::is_ref(R::get_meta_type<bool>()) );
    EXPECT_FALSE(MetaType::is_ref(R::get_meta_type<bool *>()) );
}

TEST(Reflect, node_as_pointer)
{
    // prepare
    Node* ptr = nullptr;
    auto member = std::make_unique<Member>(nullptr);

    // act
    member->set(ptr);

    // check
    EXPECT_TRUE(member->get_meta_type()->is(R::get_meta_type<void *>()) );
    EXPECT_EQ( ptr, (void*)*member );
    EXPECT_TRUE(MetaType::is_ptr(member->get_meta_type()) );
}