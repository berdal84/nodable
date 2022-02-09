#include <gtest/gtest.h>
#include <nodable/Reflect.h>
#include <nodable/Node.h>

using namespace Nodable;
using namespace Nodable::Reflect;

TEST(Reflect, is_convertible__type_to_ptr)
{
    EXPECT_TRUE( is_convertible( Type_Boolean, Type_Boolean_Ptr) );
    EXPECT_TRUE( is_convertible( Type_Double, Type_Double_Ptr) );
    EXPECT_TRUE( is_convertible( Type_String, Type_String_Ptr) );
    EXPECT_TRUE( is_convertible( Type_Any, Type_Any_Ptr) );
}

TEST(Reflect, is_convertible__ptr_to_type)
{
    EXPECT_TRUE( is_convertible( Type_Boolean_Ptr, Type_Boolean) );
    EXPECT_TRUE( is_convertible( Type_Double_Ptr, Type_Double) );
    EXPECT_TRUE( is_convertible( Type_String_Ptr, Type_String) );
    EXPECT_TRUE( is_convertible( Type_Any_Ptr, Type_Any) );
}

TEST(Reflect, is_convertible__compatible_types)
{
    EXPECT_TRUE( is_convertible( Type_Any, Type_Double) );
    EXPECT_TRUE( is_convertible( Type_Any, Type_String) );
    EXPECT_TRUE( is_convertible( Type_Any, Type_Boolean) );
    EXPECT_TRUE( is_convertible( Type_Any, Type_Any) );

    EXPECT_TRUE( is_convertible( Type_Double, Type_Any) );
    EXPECT_TRUE( is_convertible( Type_String, Type_Any) );
    EXPECT_TRUE( is_convertible( Type_Boolean, Type_Any) );
    EXPECT_TRUE( is_convertible( Type_Any, Type_Any) );
}

TEST(Reflect, is_convertible__incompatible_types)
{
    EXPECT_FALSE( is_convertible( Type_Boolean, Type_Double) );
    EXPECT_FALSE( is_convertible( Type_Double, Type_Boolean) );

    EXPECT_FALSE( is_convertible( Type_Boolean, Type_String) );
    EXPECT_FALSE( is_convertible( Type_String, Type_Boolean) );

    EXPECT_FALSE( is_convertible( Type_Double, Type_String) );
    EXPECT_FALSE( is_convertible( Type_String, Type_Double) );
}

TEST(Reflect, is_pointer)
{
    EXPECT_TRUE( is_pointer( Type_Boolean_Ptr) );
    EXPECT_FALSE( is_pointer( Type_Boolean_Ref) );
    EXPECT_FALSE( is_pointer( Type_Boolean) );

    EXPECT_TRUE( is_pointer( Type_String_Ptr) );
    EXPECT_FALSE( is_pointer( Type_String_Ref) );
    EXPECT_FALSE( is_pointer( Type_String) );

    EXPECT_TRUE( is_pointer( Type_Double_Ptr) );
    EXPECT_FALSE( is_pointer( Type_Double_Ref) );
    EXPECT_FALSE( is_pointer( Type_Double) );

    EXPECT_TRUE( is_pointer( Type_Any_Ptr) );
    EXPECT_FALSE( is_pointer( Type_Any_Ref) );
    EXPECT_FALSE( is_pointer( Type_Any) );
}

TEST(Reflect, is_reference)
{
    EXPECT_FALSE( is_reference( Type_Boolean_Ptr) );
    EXPECT_TRUE( is_reference( Type_Boolean_Ref) );
    EXPECT_FALSE( is_reference( Type_Boolean) );

    EXPECT_FALSE( is_reference( Type_String_Ptr) );
    EXPECT_TRUE( is_reference( Type_String_Ref) );
    EXPECT_FALSE( is_reference( Type_String) );

    EXPECT_FALSE( is_reference( Type_Double_Ptr) );
    EXPECT_TRUE( is_reference( Type_Double_Ref) );
    EXPECT_FALSE( is_reference( Type_Double) );

    EXPECT_FALSE( is_reference( Type_Any_Ptr) );
    EXPECT_TRUE( is_reference( Type_Any_Ref) );
    EXPECT_FALSE( is_reference( Type_Any) );
}

TEST(Reflect, node_as_pointer)
{
    // prepare
    Node   node;
    Member member;

    // act
    member.set(&node );

    // check
    EXPECT_EQ(member.getType(), Type_Pointer );
    EXPECT_EQ( &node, (Node*)(member) );
    EXPECT_TRUE( is_pointer( member.getType()) );
}