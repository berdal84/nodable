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
    EXPECT_TRUE( is_convertible(Type_Unknown, Type_Unknown_Ptr) );
}

TEST(Reflect, is_convertible__ptr_to_type)
{
    EXPECT_TRUE( is_convertible( Type_Boolean_Ptr, Type_Boolean) );
    EXPECT_TRUE( is_convertible( Type_Double_Ptr, Type_Double) );
    EXPECT_TRUE( is_convertible( Type_String_Ptr, Type_String) );
    EXPECT_TRUE( is_convertible(Type_Unknown_Ptr, Type_Unknown) );
}

TEST(Reflect, is_convertible__compatible_types)
{
    EXPECT_TRUE( is_convertible(Type_Unknown, Type_Double) );
    EXPECT_TRUE( is_convertible(Type_Unknown, Type_String) );
    EXPECT_TRUE( is_convertible(Type_Unknown, Type_Boolean) );
    EXPECT_TRUE( is_convertible(Type_Unknown, Type_Unknown) );

    EXPECT_TRUE( is_convertible(Type_Double, Type_Unknown) );
    EXPECT_TRUE( is_convertible(Type_String, Type_Unknown) );
    EXPECT_TRUE( is_convertible(Type_Boolean, Type_Unknown) );
    EXPECT_TRUE( is_convertible(Type_Unknown, Type_Unknown) );
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

    EXPECT_TRUE( is_pointer( Type_Unknown_Ptr) );
    EXPECT_FALSE( is_pointer( Type_Unknown_Ref) );
    EXPECT_FALSE( is_pointer(Type_Unknown) );
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

    EXPECT_FALSE( is_reference( Type_Unknown_Ptr) );
    EXPECT_TRUE( is_reference( Type_Unknown_Ref) );
    EXPECT_FALSE( is_reference(Type_Unknown) );
}

TEST(Reflect, node_as_pointer)
{
    // prepare
    Node   node;
    Member member(nullptr);

    // act
    member.set(&node );

    // check
    EXPECT_EQ(member.get_type(), Type_Pointer );
    EXPECT_EQ( &node, (Node*)(member) );
    EXPECT_TRUE( is_pointer(member.get_type()) );
}