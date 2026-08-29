#include <gtest/gtest.h>
#include "tools/core/reflection/Type_Descriptor.h"
using namespace tools;

TEST(Type_Descriptor, no_arg_fct)
{
    Type_Descriptor f;
    f.init<bool()>("fct");
    EXPECT_EQ( f.arg_count(), 0);
}

TEST(Type_Descriptor, push_single_arg)
{
    Type_Descriptor f;
    f.init<bool(double)>("fct");

    EXPECT_EQ(f.arg_count(), 1);
    EXPECT_TRUE(f.return_type()->is<bool>());
    EXPECT_TRUE(f.arg().at(0).type->is<double>() );
}

TEST(Type_Descriptor, push_two_args)
{
    Type_Descriptor f;
    f.init<bool(double)>("fct");

    EXPECT_EQ(f.arg_count(), 1);
    EXPECT_TRUE(f.return_type()->is<bool>());
    EXPECT_TRUE(f.arg().at(0).type->is<double>() );
    EXPECT_TRUE(f.arg().at(0).type->is<double>() );
}

TEST(Type_Descriptor, match_check_for_arg_count)
{
    Type_Descriptor f, g;
    f.init<bool(bool)>("fct");
    g.init<bool(bool, bool)>("fct");

    EXPECT_EQ(g.is_compatible(&f), false);
    EXPECT_EQ(f.is_compatible(&g), false);
}

TEST(Type_Descriptor, push_args_template_0)
{
    Type_Descriptor f, g;
    f.init<bool()>("fct");
    g.init<bool()>("fct");

    using ZeroArgs = std::tuple<>;
    g.push_args<ZeroArgs>();

    EXPECT_EQ(f.is_compatible(&g), true);
    EXPECT_EQ(g.arg_count(), 0);
}

TEST(Type_Descriptor, push_args_template_1)
{
    Type_Descriptor f, g;
    f.init<bool(double, double)>("fct");
    g.init<bool()>("fct");

    g.push_args< std::tuple<double, double> >();

    EXPECT_EQ(f.is_compatible(&g), true);
    EXPECT_EQ(g.arg_count(), 2);
}

TEST(Type_Descriptor, push_args_template_4)
{
    Type_Descriptor f, g;
    f.init<bool(double, double, double, double)>("fct");
    g.init<bool()>("fct");

    g.push_args< std::tuple<double, double, double, double> >();

    EXPECT_EQ(f.is_compatible(&g), true);
    EXPECT_EQ(g.arg_count(), 4);
}

TEST(TypeDescriptor, is_integer)
{
    EXPECT_TRUE( type_get<i8_t>()->is_integer() );
    EXPECT_TRUE( type_get<i16_t>()->is_integer() );
    EXPECT_TRUE( type_get<i32_t>()->is_integer() );
    EXPECT_TRUE( type_get<i64_t>()->is_integer() );
    EXPECT_TRUE( type_get<u8_t>()->is_integer() );
    EXPECT_TRUE( type_get<u16_t>()->is_integer() );
    EXPECT_TRUE( type_get<u32_t>()->is_integer() );
    EXPECT_TRUE( type_get<u64_t>()->is_integer() );

    EXPECT_FALSE( type_get<double>()->is_integer() );
    EXPECT_FALSE( type_get<float>()->is_integer() );
    EXPECT_FALSE( type_get<bdc::String>()->is_integer() );
}

TEST(TypeDescriptor, is_floating_point)
{
    EXPECT_TRUE( type_get<double>()->is_floating_point() );
    EXPECT_TRUE( type_get<float>()->is_floating_point() );

    EXPECT_FALSE( type_get<i8_t>()->is_floating_point() );
    EXPECT_FALSE( type_get<i16_t>()->is_floating_point() );
    EXPECT_FALSE( type_get<i32_t>()->is_floating_point() );
    EXPECT_FALSE( type_get<i64_t>()->is_floating_point() );
    EXPECT_FALSE( type_get<u8_t>()->is_floating_point() );
    EXPECT_FALSE( type_get<u16_t>()->is_floating_point() );
    EXPECT_FALSE( type_get<u32_t>()->is_floating_point() );
    EXPECT_FALSE( type_get<u64_t>()->is_floating_point() );
    EXPECT_FALSE( type_get<bdc::String>()->is_integer() );
}

TEST(TypeDescriptor, pass_by_ref_argument )
{
    Type_Descriptor f;
    f.init<double(double &, double)>("=");
    EXPECT_TRUE(f.arg_at(0).pass_by_ref);
    EXPECT_FALSE(f.arg_at(1).pass_by_ref);
}
