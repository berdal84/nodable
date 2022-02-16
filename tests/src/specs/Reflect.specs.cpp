#include <gtest/gtest.h>
#include <nodable/R.h>
#include <nodable/Node.h>

using namespace Nodable;
using namespace Nodable::R;

TEST(Reflect, is_convertible__type_to_ptr)
{
    EXPECT_TRUE( is_convertible( Type::Boolean, add_ptr(Type::Boolean) ) );
    EXPECT_TRUE( is_convertible( Type::Double, add_ptr(Type::Double) ) );
    EXPECT_TRUE( is_convertible( Type::String, add_ptr(Type::String) ) );
    EXPECT_TRUE( is_convertible(Type::Unknown, add_ptr(Type::Unknown)) );
}

TEST(Reflect, is_convertible__ptr_to_type)
{
    EXPECT_TRUE( is_convertible( add_ptr(Type::Boolean), Type::Boolean) );
    EXPECT_TRUE( is_convertible( add_ptr(Type::Double), Type::Double) );
    EXPECT_TRUE( is_convertible( add_ptr(Type::String), Type::String) );
    EXPECT_TRUE( is_convertible( add_ptr(Type::Unknown), Type::Unknown) );
}

TEST(Reflect, is_convertible__compatible_types)
{
    EXPECT_TRUE( is_convertible(Type::Unknown, Type::Double) );
    EXPECT_TRUE( is_convertible(Type::Unknown, Type::String) );
    EXPECT_TRUE( is_convertible(Type::Unknown, Type::Boolean) );
    EXPECT_TRUE( is_convertible(Type::Unknown, Type::Void) );
    EXPECT_TRUE( is_convertible(Type::Unknown, Type::Unknown) );

    EXPECT_TRUE( is_convertible(Type::Void, Type::Unknown) );
    EXPECT_TRUE( is_convertible(Type::Double, Type::Unknown) );
    EXPECT_TRUE( is_convertible(Type::String, Type::Unknown) );
    EXPECT_TRUE( is_convertible(Type::Boolean, Type::Unknown) );
    EXPECT_TRUE( is_convertible(Type::Unknown, Type::Unknown) );
}

TEST(Reflect, is_convertible__incompatible_types)
{
    EXPECT_FALSE( is_convertible( Type::Boolean, Type::Double) );
    EXPECT_FALSE( is_convertible( Type::Double, Type::Boolean) );

    EXPECT_FALSE( is_convertible( Type::Boolean, Type::String) );
    EXPECT_FALSE( is_convertible( Type::String, Type::Boolean) );

    EXPECT_FALSE( is_convertible( Type::Double, Type::String) );
    EXPECT_FALSE( is_convertible( Type::String, Type::Double) );
}

TEST(Reflect, is_ptr)
{
    EXPECT_TRUE(is_ptr( binary_or(Type::Boolean, Type::Ptr)) );
    EXPECT_FALSE(is_ptr( add_ref(Type::Boolean)) );
    EXPECT_FALSE(is_ptr(Type::Boolean) );

    EXPECT_TRUE(is_ptr(binary_or(Type::String, Type::Ptr)) );
    EXPECT_FALSE(is_ptr(binary_or(Type::String, Type::Ref) ));
    EXPECT_FALSE(is_ptr(Type::String) );

    EXPECT_TRUE(is_ptr( add_ptr(Type::Double)) );
    EXPECT_FALSE(is_ptr( add_ref(Type::Double)) );
    EXPECT_FALSE(is_ptr(Type::Double) );

    EXPECT_TRUE(is_ptr( add_ptr(Type::Void)) );
    EXPECT_FALSE(is_ptr(Type::Void) );

    EXPECT_TRUE(is_ptr( Type::Ptr ) );
    EXPECT_TRUE(is_ptr( Type::DblPtr) );
}

TEST(Reflect, is_ref)
{
    EXPECT_FALSE(is_ref( add_ptr(Type::Boolean)) );
    EXPECT_TRUE(is_ref( add_ref(Type::Boolean)) );
    EXPECT_FALSE(is_ref(Type::Boolean) );

    EXPECT_FALSE(is_ref( add_ptr(Type::String)) );
    EXPECT_TRUE(is_ref( add_ref(Type::String)));
    EXPECT_FALSE(is_ref(Type::String) );

    EXPECT_FALSE(is_ref( add_ptr(Type::Double)) );
    EXPECT_TRUE(is_ref( add_ref(Type::Double)) );
    EXPECT_FALSE(is_ref(Type::Double) );

    EXPECT_FALSE(is_ref( add_ptr(Type::Void)) );
    EXPECT_FALSE(is_ref(Type::Void) );
}

TEST(Reflect, node_as_pointer)
{
    // prepare
    Node* ptr = nullptr;
    auto member = std::make_unique<Member>(nullptr);

    // act
    member->set(ptr);

    // check
    EXPECT_EQ(member->get_type(), add_ptr(Type::Void) );
    EXPECT_EQ( ptr, (void*)*member );
    EXPECT_TRUE(is_ptr(member->get_type()) );
}