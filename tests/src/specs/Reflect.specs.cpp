#include <gtest/gtest.h>
#include <nodable/R.h>
#include <nodable/Node.h>

using namespace Nodable;
using namespace Nodable::R;

TEST(Reflect, is_convertible__type_to_ptr)
{
    EXPECT_TRUE( Type::is_convertible( R::get_type<bool>(),R::get_type<bool*>() ) );
    EXPECT_TRUE( Type::is_convertible( R::get_type<double>(), R::get_type<double*>()) );
    EXPECT_TRUE( Type::is_convertible( R::get_type<std::string>(), R::get_type<std::string*>())  );
}

TEST(Reflect, is_convertible__ptr_to_type)
{
    EXPECT_TRUE( Type::is_convertible( R::get_type<bool*>(), R::get_type<bool>()) );
    EXPECT_TRUE( Type::is_convertible( R::get_type<double*>(), R::get_type<double>()) );
    EXPECT_TRUE( Type::is_convertible( R::get_type<std::string*>(), R::get_type<std::string>()) );
    EXPECT_TRUE( Type::is_convertible( R::Type::s_unknown, R::Type::s_unknown ));
}

TEST(Reflect, is_convertible__compatible_types)
{
    EXPECT_TRUE( Type::is_convertible(R::Type::s_unknown, R::get_type<double>()) );
    EXPECT_TRUE( Type::is_convertible(R::Type::s_unknown, R::get_type<std::string>()) );
    EXPECT_TRUE( Type::is_convertible(R::Type::s_unknown, R::get_type<bool>()) );
    EXPECT_TRUE( Type::is_convertible(R::Type::s_unknown, R::get_type<void>()) );
    EXPECT_TRUE( Type::is_convertible(R::Type::s_unknown, R::Type::s_unknown) );

    EXPECT_TRUE( Type::is_convertible(R::get_type<void>(), R::Type::s_unknown) );
    EXPECT_TRUE( Type::is_convertible(R::get_type<double>(), R::Type::s_unknown) );
    EXPECT_TRUE( Type::is_convertible(R::get_type<std::string>(), R::Type::s_unknown) );
    EXPECT_TRUE( Type::is_convertible(R::get_type<bool>(), R::Type::s_unknown) );
    EXPECT_TRUE( Type::is_convertible(R::Type::s_unknown, R::Type::s_unknown) );
}

TEST(Reflect, is_convertible__incompatible_types)
{
    EXPECT_FALSE( Type::is_convertible( R::get_type<bool>(), R::get_type<double>()) );
    EXPECT_FALSE( Type::is_convertible( R::get_type<double>(), R::get_type<bool>()) );

    EXPECT_FALSE( Type::is_convertible( R::get_type<bool>(), R::get_type<std::string>()) );
    EXPECT_FALSE( Type::is_convertible( R::get_type<std::string>(), R::get_type<bool>()) );

    EXPECT_FALSE( Type::is_convertible( R::get_type<double>(), R::get_type<std::string>()) );
    EXPECT_FALSE( Type::is_convertible( R::get_type<std::string>(), R::get_type<double>()) );
}

TEST(Reflect, is_ptr)
{
    EXPECT_FALSE(Type::is_ptr( R::get_type<bool&>()) );
    EXPECT_FALSE(Type::is_ptr(R::get_type<bool>()) );
    EXPECT_TRUE(Type::is_ptr(R::get_type<bool*>()) );
}

TEST(Reflect, is_ref)
{
    EXPECT_TRUE(Type::is_ref(R::get_type<bool&>()) );
    EXPECT_FALSE(Type::is_ref(R::get_type<bool>()) );
    EXPECT_FALSE(Type::is_ref(R::get_type<bool*>()) );
}

TEST(Reflect, node_as_pointer)
{
    // prepare
    Node* ptr = nullptr;
    auto member = std::make_unique<Member>(nullptr);

    // act
    member->set(ptr);

    // check
    EXPECT_TRUE( member->get_type()->equals( R::get_type<void*>()) );
    EXPECT_EQ( ptr, (void*)*member );
    EXPECT_TRUE(Type::is_ptr(member->get_type()) );
}