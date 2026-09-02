#include <gtest/gtest.h>

#include "index.h"
#include "../Log.h"

using namespace tools;

TEST(Reflection, is_convertible__type_to_ptr)
{
    EXPECT_FALSE(type_is_implicitly_convertible(type_get<bool>(), type_get<bool *>()) );
    EXPECT_FALSE(type_is_implicitly_convertible(type_get<double>(), type_get<double *>()) );
    EXPECT_FALSE(type_is_implicitly_convertible(type_get<bdc::String>(), type_get<bdc::String *>())  );
    EXPECT_TRUE(type_is_implicitly_convertible(type_get<bdc::String>(), type_get<bdc::String&>())  );
}

TEST(Reflection, is_convertible__ptr_to_type)
{
    EXPECT_FALSE(type_is_implicitly_convertible(type_get<bool *>(), type_get<bool>()) );
    EXPECT_FALSE(type_is_implicitly_convertible(type_get<double *>(), type_get<double>()) );
    EXPECT_FALSE(type_is_implicitly_convertible(type_get<bdc::String *>(), type_get<bdc::String>()) );
    EXPECT_TRUE(type_is_implicitly_convertible(type_any(), type_any()));
}

TEST(Reflection, is_convertible__compatible_types)
{
    EXPECT_TRUE(type_is_implicitly_convertible(type_any(), type_get<double>()) );
    EXPECT_TRUE(type_is_implicitly_convertible(type_any(), type_get<bdc::String>()) );
    EXPECT_TRUE(type_is_implicitly_convertible(type_any(), type_get<bool>()) );
    EXPECT_TRUE(type_is_implicitly_convertible(type_any(), type_get<void>()) );
    EXPECT_TRUE(type_is_implicitly_convertible(type_any(), type_any()) );

    EXPECT_TRUE(type_is_implicitly_convertible(type_get<void>(), type_any()) );
    EXPECT_TRUE(type_is_implicitly_convertible(type_get<double>(), type_any()) );
    EXPECT_TRUE(type_is_implicitly_convertible(type_get<bdc::String>(), type_any()) );
    EXPECT_TRUE(type_is_implicitly_convertible(type_get<bool>(), type_any()) );
    EXPECT_TRUE(type_is_implicitly_convertible(type_any(), type_any()) );
}

TEST(Reflection, is_convertible__incompatible_types)
{
    EXPECT_FALSE(type_is_implicitly_convertible(type_get<bool>(), type_get<double>()) );
    EXPECT_FALSE(type_is_implicitly_convertible(type_get<double>(), type_get<bool>()) );

    EXPECT_FALSE(type_is_implicitly_convertible(type_get<bool>(), type_get<bdc::String>()) );
    EXPECT_FALSE(type_is_implicitly_convertible(type_get<bdc::String>(), type_get<bool>()) );

    EXPECT_FALSE(type_is_implicitly_convertible(type_get<double>(), type_get<bdc::String>()) );
    EXPECT_FALSE(type_is_implicitly_convertible(type_get<bdc::String>(), type_get<double>()) );
}

TEST(Reflection, is_ptr)
{
    EXPECT_FALSE( type_get<bool>()->is_ptr() );
    EXPECT_TRUE ( type_get<bool*>()->is_ptr() );
}

TEST(Reflection, is_child_of)
{
    memory_manager_init();

    class Base {};
    class Derived: public Base {};

    Type_Initializer<Derived>("Derived").extends<Base>();

    EXPECT_TRUE ( type_get<Derived>()->class_is_child_of<Base>() );
    EXPECT_FALSE( type_get<Base>()->class_is_child_of<Derived>() );

    memory_manager_shutdown();
}

TEST(Reflection, pass_by_ref)
{
   memory_manager_init();

   Type_Descriptor type;
   type_init<void(double &d)>(&type, "function");
   EXPECT_TRUE( type.function.args[0].pass_by_ref );

   memory_manager_shutdown();
}
