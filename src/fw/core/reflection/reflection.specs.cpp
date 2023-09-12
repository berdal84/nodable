#include <gtest/gtest.h>

// Hack to avoid adding friend to classes
#define private public
#define protected public

#include "reflection"
#include "../log.h"

using namespace fw;

TEST(Reflection, is_convertible__type_to_ptr)
{
    EXPECT_FALSE(type::is_implicitly_convertible(type::get<bool>(), type::get<bool *>()) );
    EXPECT_FALSE(type::is_implicitly_convertible(type::get<double>(), type::get<double *>()) );
    EXPECT_FALSE(type::is_implicitly_convertible(type::get<std::string>(), type::get<std::string *>())  );
    EXPECT_TRUE(type::is_implicitly_convertible(type::get<std::string>(), type::get<std::string&>())  );
}

TEST(Reflection, is_convertible__ptr_to_type)
{
    EXPECT_FALSE(type::is_implicitly_convertible(type::get<bool *>(), type::get<bool>()) );
    EXPECT_FALSE(type::is_implicitly_convertible(type::get<double *>(), type::get<double>()) );
    EXPECT_FALSE(type::is_implicitly_convertible(type::get<std::string *>(), type::get<std::string>()) );
    EXPECT_TRUE(type::is_implicitly_convertible(type::any(), type::any()));
}

TEST(Reflection, is_convertible__compatible_types)
{
    EXPECT_TRUE(type::is_implicitly_convertible(type::any(), type::get<double>()) );
    EXPECT_TRUE(type::is_implicitly_convertible(type::any(), type::get<std::string>()) );
    EXPECT_TRUE(type::is_implicitly_convertible(type::any(), type::get<bool>()) );
    EXPECT_TRUE(type::is_implicitly_convertible(type::any(), type::get<void>()) );
    EXPECT_TRUE(type::is_implicitly_convertible(type::any(), type::any()) );

    EXPECT_TRUE(type::is_implicitly_convertible(type::get<void>(), type::any()) );
    EXPECT_TRUE(type::is_implicitly_convertible(type::get<double>(), type::any()) );
    EXPECT_TRUE(type::is_implicitly_convertible(type::get<std::string>(), type::any()) );
    EXPECT_TRUE(type::is_implicitly_convertible(type::get<bool>(), type::any()) );
    EXPECT_TRUE(type::is_implicitly_convertible(type::any(), type::any()) );
}

TEST(Reflection, is_convertible__incompatible_types)
{
    EXPECT_FALSE(type::is_implicitly_convertible(type::get<bool>(), type::get<double>()) );
    EXPECT_FALSE(type::is_implicitly_convertible(type::get<double>(), type::get<bool>()) );

    EXPECT_FALSE(type::is_implicitly_convertible(type::get<bool>(), type::get<std::string>()) );
    EXPECT_FALSE(type::is_implicitly_convertible(type::get<std::string>(), type::get<bool>()) );

    EXPECT_FALSE(type::is_implicitly_convertible(type::get<double>(), type::get<std::string>()) );
    EXPECT_FALSE(type::is_implicitly_convertible(type::get<std::string>(), type::get<double>()) );
}

TEST(Reflection, is_ptr)
{
    EXPECT_FALSE(type::is_ptr(type::get<bool>()));
    EXPECT_TRUE(type::is_ptr(type::get<bool*>()));
}

TEST(Reflection, variant_and_id)
{
    // prepare
    struct A {};
    PoolID<A> id{42};
    variant value;

    // act
    value.set( id );

    // check
    EXPECT_TRUE( value.get_type()->equals( type::get<PoolID<A>>() ) );
    EXPECT_EQ( id, (PoolID<A>)value );
}

TEST(Reflection, is_child_of)
{
    class Base {};
    class Derived: public Base {};

    registration::push_class<Derived>("Derived").extends<Base>();

    EXPECT_TRUE(type::get<Derived>()->is_child_of<Base>());
    EXPECT_FALSE(type::get<Base>()->is_child_of<Derived>());
}

