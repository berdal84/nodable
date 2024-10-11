#include <gtest/gtest.h>
#include "tools/core/reflection/FuncType.h"
using namespace tools;

TEST(func_type_builder, no_arg_fct)
{
    auto no_arg_fct = FuncTypeBuilder<bool()>("fct").construct();
    EXPECT_EQ(no_arg_fct.get_arg_count(), 0);
}

TEST(func_type_builder, push_single_arg)
{
    FuncType single_arg_fct = FuncTypeBuilder<bool(double)>("fct").construct();

    EXPECT_EQ(single_arg_fct.get_arg_count(), 1);
    EXPECT_EQ(single_arg_fct.get_return_type(), type::get<bool>());
    EXPECT_EQ(single_arg_fct.get_args().at(0).m_type, type::get<double>());
}

TEST(func_type_builder, push_two_args)
{
    auto two_arg_fct = FuncTypeBuilder<bool(double, double)>("fct").construct();

    EXPECT_EQ(two_arg_fct.get_arg_count(), 2);
    EXPECT_EQ(two_arg_fct.get_return_type(), type::get<bool>());
    EXPECT_EQ(two_arg_fct.get_args().at(0).m_type, type::get<double>());
    EXPECT_EQ(two_arg_fct.get_args().at(1).m_type, type::get<double>());
}

TEST(func_type_builder, match_check_for_arg_count)
{
    FuncType single_arg_fct = FuncTypeBuilder<bool(bool)>("fct").construct();
    FuncType two_arg_fct    = FuncTypeBuilder<bool(bool, bool)>("fct").construct();

    EXPECT_EQ(two_arg_fct.is_compatible(&single_arg_fct), false);
    EXPECT_EQ(single_arg_fct.is_compatible(&two_arg_fct), false);
}

TEST(func_type_builder, match_check_identifier)
{
    FuncType two_arg_fct          = FuncTypeBuilder<bool(bool, bool)>("fct").construct();
    FuncType two_arg_fct_modified = FuncTypeBuilder<bool()>("fct").construct();

    two_arg_fct_modified.push_arg(type::get<double>() );
    two_arg_fct_modified.push_arg(type::get<double>() );

    EXPECT_EQ(two_arg_fct.is_compatible(&two_arg_fct_modified), false);
    EXPECT_EQ(two_arg_fct_modified.is_compatible(&two_arg_fct), false);
}

TEST(func_type_builder, match_check_absence_of_arg)
{
    FuncType two_arg_fct              = FuncTypeBuilder<bool(bool, bool)>("fct").construct();
    FuncType two_arg_fct_without_args = FuncTypeBuilder<bool()>("fct").construct();

    EXPECT_EQ(two_arg_fct.is_compatible(&two_arg_fct_without_args), false);
    EXPECT_EQ(two_arg_fct_without_args.is_compatible(&two_arg_fct), false);
}

TEST(func_type_builder, push_args_template_0)
{
    auto ref = FuncTypeBuilder<bool()>("fct").construct();
    auto fct = FuncTypeBuilder<bool()>("fct").construct();

    using Args = std::tuple<>; // create arg tuple
    fct.push_args<Args>(); // push those args to signature

    EXPECT_EQ(ref.is_compatible(&fct), true);
    EXPECT_EQ(fct.get_arg_count(), 0);
}

TEST(func_type_builder, push_args_template_1)
{
    auto ref = FuncTypeBuilder<bool(double, double)>("fct").construct();
    auto fct = FuncTypeBuilder<bool()>("fct").construct();

    fct.push_args< std::tuple<double, double> >();

    EXPECT_EQ(ref.is_compatible(&fct), true);
    EXPECT_EQ(fct.get_arg_count(), 2);
}

TEST(func_type_builder, push_args_template_4)
{
    auto ref = FuncTypeBuilder<bool(double, double, double, double)>("fct").construct();
    auto fct = FuncTypeBuilder<bool()>("fct").construct();
    fct.push_args< std::tuple<double, double, double, double> >();

    EXPECT_EQ(ref.is_compatible(&fct), true);
    EXPECT_EQ(fct.get_arg_count(), 4);
}
