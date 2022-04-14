#include <gtest/gtest.h>

#include <nodable/core/Invokable.h>

using namespace Nodable;

TEST( Function_Signature, no_arg_fct)
{
    auto no_arg_fct = Signature::from_type<bool()>::as_function("fct");

    EXPECT_EQ(no_arg_fct->get_arg_count(), 0);
    EXPECT_EQ(no_arg_fct->get_arg_count(), 0);
}

TEST( Function_Signature, push_single_arg)
{
    Signature* single_arg_fct = Signature::from_type<bool(double)>::as_function("fct");

    EXPECT_EQ(single_arg_fct->get_arg_count(), 1);
    EXPECT_EQ(single_arg_fct->get_return_type(), type::get<bool>());
    EXPECT_EQ(single_arg_fct->get_args().at(0).m_type, type::get<double>());
}

TEST( Function_Signature, push_two_args)
{
    auto two_arg_fct = Signature::from_type<bool(double, double)>::as_function("fct");

    EXPECT_EQ(two_arg_fct->get_arg_count(), 2);
    EXPECT_EQ(two_arg_fct->get_return_type(), type::get<bool>());
    EXPECT_EQ(two_arg_fct->get_args().at(0).m_type, type::get<double>());
    EXPECT_EQ(two_arg_fct->get_args().at(1).m_type, type::get<double>());
}

TEST( Function_Signature, match_check_for_arg_count)
{
    Signature* single_arg_fct = Signature
    ::from_type<bool(bool)>
    ::as_function("fct");

    Signature* two_arg_fct = Signature
    ::from_type<bool(bool, bool)>
    ::as_function("fct");

    EXPECT_EQ(two_arg_fct->is_compatible(single_arg_fct), false);
    EXPECT_EQ(single_arg_fct->is_compatible(two_arg_fct), false);
}

TEST( Function_Signature, match_check_identifier)
{
    Signature* two_arg_fct = Signature::from_type<bool(bool, bool)>::as_function("fct");

    Signature* two_arg_fct_modified = Signature::from_type<bool()>::as_function("fct");

    two_arg_fct_modified->push_arg(type::get<double>() );
    two_arg_fct_modified->push_arg(type::get<double>() );

    EXPECT_EQ(two_arg_fct->is_compatible(two_arg_fct_modified), false);
    EXPECT_EQ(two_arg_fct_modified->is_compatible(two_arg_fct), false);
}

TEST( Function_Signature, match_check_absence_of_arg)
{
    Signature* two_arg_fct = Signature::from_type<bool(bool, bool)>::as_function("fct");

    Signature* two_arg_fct_without_args = Signature::from_type<bool()>::as_function("fct");

    EXPECT_EQ(two_arg_fct->is_compatible(two_arg_fct_without_args), false);
    EXPECT_EQ(two_arg_fct_without_args->is_compatible(two_arg_fct), false);
}

TEST( Function_Signature, push_args_template_0)
{
    auto ref = Signature::from_type<bool()>::as_function("fct");

    auto fct = Signature::from_type<bool()>::as_function("fct");

    using Args = std::tuple<>; // create arg tuple
    fct->push_args<Args>(); // push those args to signature

    EXPECT_EQ(ref->is_compatible(fct), true);
    EXPECT_EQ(fct->get_arg_count(), 0);
}

TEST( Function_Signature, push_args_template_1)
{
    auto ref = Signature::from_type<bool(double, double)>::as_function("fct");

    auto fct = Signature::from_type<bool()>::as_function("fct");

    fct->push_args< std::tuple<double, double> >();

    EXPECT_EQ(ref->is_compatible(fct), true);
    EXPECT_EQ(fct->get_arg_count(), 2);
}


TEST( Function_Signature, push_args_template_4)
{
    auto ref = Signature::from_type<bool(double, double, double, double)>::as_function("fct");

    auto fct = Signature::from_type<bool()>::as_function("fct");
    fct->push_args< std::tuple<double, double, double, double> >();

    EXPECT_EQ(ref->is_compatible(fct), true);
    EXPECT_EQ(fct->get_arg_count(), 4);
}
