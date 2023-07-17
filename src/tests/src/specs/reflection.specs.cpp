#include <gtest/gtest.h>
#include "fw/core/reflection/reflection"
#include "nodable/core/Node.h"

using namespace fw;

TEST(Reflection, is_convertible__type_to_ptr)
{
    EXPECT_TRUE(type::is_implicitly_convertible(type::get<bool>(), type::get<bool *>()) );
    EXPECT_TRUE(type::is_implicitly_convertible(type::get<double>(), type::get<double *>()) );
    EXPECT_TRUE(type::is_implicitly_convertible(type::get<std::string>(), type::get<std::string *>())  );
}

TEST(Reflection, is_convertible__ptr_to_type)
{
    EXPECT_TRUE(type::is_implicitly_convertible(type::get<bool *>(), type::get<bool>()) );
    EXPECT_TRUE(type::is_implicitly_convertible(type::get<double *>(), type::get<double>()) );
    EXPECT_TRUE(type::is_implicitly_convertible(type::get<std::string *>(), type::get<std::string>()) );
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
    EXPECT_FALSE(type::is_ptr(type::get<bool>()) );
    EXPECT_TRUE(type::is_ptr(type::get<bool *>()) );
}

TEST(Reflection, node_as_pointer)
{
    // prepare
    ndbl::Node node;
    variant v;

    // act
    v.set(&node);

    // check
    EXPECT_EQ(v.get_type(), type::get<void*>() );
    EXPECT_EQ(&node, (ndbl::Node*)v );
    EXPECT_TRUE(v.get_type()->is_ptr());
}