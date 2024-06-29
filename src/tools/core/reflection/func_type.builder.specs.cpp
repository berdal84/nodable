#include <gtest/gtest.h>
#include "tools/core/reflection/func_type.h"
using namespace tools;

TEST(func_type_builder, no_arg_fct)
{
    auto no_arg_fct = func_type_builder<bool()>::with_id("fct");
    EXPECT_EQ(no_arg_fct->get_arg_count(), 0);
}

TEST(func_type_builder, push_single_arg)
{
    func_type* single_arg_fct = func_type_builder<bool(double)>::with_id("fct");

    EXPECT_EQ(single_arg_fct->get_arg_count(), 1);
    EXPECT_EQ(single_arg_fct->get_return_type(), type::get<bool>());
    EXPECT_EQ(single_arg_fct->get_args().at(0).m_type, type::get<double>());
}

TEST(func_type_builder, push_two_args)
{
    auto two_arg_fct = func_type_builder<bool(double, double)>::with_id("fct");

    EXPECT_EQ(two_arg_fct->get_arg_count(), 2);
    EXPECT_EQ(two_arg_fct->get_return_type(), type::get<bool>());
    EXPECT_EQ(two_arg_fct->get_args().at(0).m_type, type::get<double>());
    EXPECT_EQ(two_arg_fct->get_args().at(1).m_type, type::get<double>());
}

TEST(func_type_builder, match_check_for_arg_count)
{
    func_type* single_arg_fct = func_type_builder<bool(bool)>::with_id("fct");
    func_type* two_arg_fct    = func_type_builder<bool(bool, bool)>::with_id("fct");

    EXPECT_EQ(two_arg_fct->is_compatible(single_arg_fct), false);
    EXPECT_EQ(single_arg_fct->is_compatible(two_arg_fct), false);
}

TEST(func_type_builder, match_check_identifier)
{
    func_type* two_arg_fct          = func_type_builder<bool(bool, bool)>::with_id("fct");
    func_type* two_arg_fct_modified = func_type_builder<bool()>::with_id("fct");

    two_arg_fct_modified->push_arg(type::get<double>() );
    two_arg_fct_modified->push_arg(type::get<double>() );

    EXPECT_EQ(two_arg_fct->is_compatible(two_arg_fct_modified), false);
    EXPECT_EQ(two_arg_fct_modified->is_compatible(two_arg_fct), false);
}

TEST(func_type_builder, match_check_absence_of_arg)
{
    func_type* two_arg_fct              = func_type_builder<bool(bool, bool)>::with_id("fct");
    func_type* two_arg_fct_without_args = func_type_builder<bool()>::with_id("fct");

    EXPECT_EQ(two_arg_fct->is_compatible(two_arg_fct_without_args), false);
    EXPECT_EQ(two_arg_fct_without_args->is_compatible(two_arg_fct), false);
}

TEST(func_type_builder, push_args_template_0)
{
    auto ref = func_type_builder<bool()>::with_id("fct");
    auto fct = func_type_builder<bool()>::with_id("fct");

    using Args = std::tuple<>; // create arg tuple
    fct->push_args<Args>(); // push those args to signature

    EXPECT_EQ(ref->is_compatible(fct), true);
    EXPECT_EQ(fct->get_arg_count(), 0);
}

TEST(func_type_builder, push_args_template_1)
{
    auto ref = func_type_builder<bool(double, double)>::with_id("fct");
    auto fct = func_type_builder<bool()>::with_id("fct");

    fct->push_args< std::tuple<double, double> >();

    EXPECT_EQ(ref->is_compatible(fct), true);
    EXPECT_EQ(fct->get_arg_count(), 2);
}

TEST(func_type_builder, push_args_template_4)
{
    auto ref = func_type_builder<bool(double, double, double, double)>::with_id("fct");
    auto fct = func_type_builder<bool()>::with_id("fct");
    fct->push_args< std::tuple<double, double, double, double> >();

    EXPECT_EQ(ref->is_compatible(fct), true);
    EXPECT_EQ(fct->get_arg_count(), 4);
}
