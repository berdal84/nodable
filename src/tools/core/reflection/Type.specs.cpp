#include <gtest/gtest.h>
#include "tools/core/reflection/Type.h"
using namespace tools;

TEST(FunctionDescriptor, no_arg_fct)
{
    const FunctionDescriptor f = FunctionDescriptor::create<bool()>("fct");
    EXPECT_EQ(f.get_arg_count(), 0);
}

TEST(FunctionDescriptor, push_single_arg)
{
    const FunctionDescriptor f = FunctionDescriptor::create<bool(double)>("fct");

    EXPECT_EQ(f.get_arg_count(), 1);
    EXPECT_TRUE(f.get_return_type()->is<bool>());
    EXPECT_TRUE(f.get_args().at(0).m_type->is<double>() );
}

TEST(FunctionDescriptor, push_two_args)
{
    const FunctionDescriptor f = FunctionDescriptor::create<bool(double)>("fct");

    EXPECT_EQ(f.get_arg_count(), 1);
    EXPECT_TRUE(f.get_return_type()->is<bool>());
    EXPECT_TRUE(f.get_args().at(0).m_type->is<double>() );
    EXPECT_TRUE(f.get_args().at(0).m_type->is<double>() );
}

TEST(FunctionDescriptor, match_check_for_arg_count)
{
    const FunctionDescriptor f = FunctionDescriptor::create<bool(bool)>("fct");
    const FunctionDescriptor g = FunctionDescriptor::create<bool(bool, bool)>("fct");

    EXPECT_EQ(g.is_compatible(&f), false);
    EXPECT_EQ(f.is_compatible(&g), false);
}

TEST(FunctionDescriptor, push_args_template_0)
{
    const FunctionDescriptor  f = FunctionDescriptor::create<bool()>("fct");
    FunctionDescriptor        g = FunctionDescriptor::create<bool()>("fct");

    using ZeroArgs = std::tuple<>;
    g.push_args<ZeroArgs>();

    EXPECT_EQ(f.is_compatible(&g), true);
    EXPECT_EQ(g.get_arg_count(), 0);
}

TEST(FunctionDescriptor, push_args_template_1)
{
    const FunctionDescriptor f = FunctionDescriptor::create<bool(double, double)>("fct");
    FunctionDescriptor       g = FunctionDescriptor::create<bool()>("fct");

    g.push_args< std::tuple<double, double> >();

    EXPECT_EQ(f.is_compatible(&g), true);
    EXPECT_EQ(g.get_arg_count(), 2);
}

TEST(FunctionDescriptor, push_args_template_4)
{
    const FunctionDescriptor f = FunctionDescriptor::create<bool(double, double, double, double)>("fct");
    FunctionDescriptor       g = FunctionDescriptor::create<bool()>("fct");

    g.push_args< std::tuple<double, double, double, double> >();

    EXPECT_EQ(f.is_compatible(&g), true);
    EXPECT_EQ(g.get_arg_count(), 4);
}
