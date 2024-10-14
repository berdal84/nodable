#include <gtest/gtest.h>

// Hack to avoid adding friend to classes
#define private public
#define protected public

#include "reflection"
#include "../log.h"

using namespace tools;

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
    EXPECT_FALSE( type::get<bool>()->is_ptr() );
    EXPECT_TRUE( type::get<bool*>()->is_ptr() );
}

TEST(Reflection, is_child_of)
{
    class Base {};
    class Derived: public Base {};

    type::Initializer<Derived>("Derived").extends<Base>();

    EXPECT_TRUE(type::get_class<Derived>()->is_child_of<Base>());
    EXPECT_FALSE(type::get_class<Base>()->is_child_of<Derived>());
}

TEST(Reflection, by_reference)
{
   ;
   auto descriptor = FunctionDescriptor::construct<void(double &d)>("function");
   EXPECT_TRUE( descriptor.get_arg(0).m_by_reference );
}
