#include <gtest/gtest.h>

#include <nodable/InvokableFunction.h>

using namespace Nodable;
using namespace Nodable::R;

TEST( Function_Signature, no_arg_fct)
{
    auto no_arg_fct = FunctionSignature::new_instance<bool()>::with_id("fct");
    EXPECT_EQ(no_arg_fct->get_arg_count(), 0);
    EXPECT_EQ(no_arg_fct->get_arg_count(), 0);
}

TEST( Function_Signature, push_single_arg)
{
    FunctionSignature* single_arg_fct = FunctionSignature::new_instance<bool(double)>::with_id("fct");
    EXPECT_EQ(single_arg_fct->get_arg_count(), 1);
    EXPECT_EQ(single_arg_fct->get_return_type()->get_type(), Type::Boolean);
    EXPECT_EQ(single_arg_fct->get_args().at(0).m_type->get_type(), Type::Double);
}

TEST( Function_Signature, push_two_args)
{
    auto two_arg_fct = FunctionSignature::new_instance<bool(double, double)>::with_id("fct");

    EXPECT_EQ(two_arg_fct->get_arg_count(), 2);
    EXPECT_EQ(two_arg_fct->get_return_type()->get_type(), Type::Boolean);
    EXPECT_EQ(two_arg_fct->get_args().at(0).m_type->get_type(), Type::Double);
    EXPECT_EQ(two_arg_fct->get_args().at(1).m_type->get_type(), Type::Double);
}

TEST( Function_Signature, match_check_for_arg_count)
{
    FunctionSignature* single_arg_fct = FunctionSignature::new_instance<bool(bool)>::with_id("fct");
    FunctionSignature* two_arg_fct = FunctionSignature::new_instance<bool(bool, bool)>::with_id("fct");

    EXPECT_EQ(two_arg_fct->match( single_arg_fct ), false);
    EXPECT_EQ(single_arg_fct->match( two_arg_fct ), false);
}

TEST( Function_Signature, match_check_identifier)
{
    FunctionSignature* two_arg_fct = FunctionSignature::new_instance<bool(bool, bool)>::with_id("fct");

    FunctionSignature* two_arg_fct_modified = FunctionSignature::new_instance<bool()>::with_id("fct");
    two_arg_fct_modified->push_arg(R::get_meta_type<double>() );
    two_arg_fct_modified->push_arg(R::get_meta_type<double>() );

    EXPECT_EQ(two_arg_fct->match( two_arg_fct_modified ), false);
    EXPECT_EQ(two_arg_fct_modified->match( two_arg_fct ), false);
}

TEST( Function_Signature, match_check_absence_of_arg)
{
    FunctionSignature* two_arg_fct = FunctionSignature::new_instance<bool(bool, bool)>::with_id("fct");

    FunctionSignature* two_arg_fct_without_args = FunctionSignature::new_instance<bool()>::with_id("fct");

    EXPECT_EQ(two_arg_fct->match( two_arg_fct_without_args ), false);
    EXPECT_EQ(two_arg_fct_without_args->match( two_arg_fct ), false);
}

TEST( Function_Signature, push_args_template_0)
{
    auto ref = FunctionSignature::new_instance<bool()>::with_id("fct");

    auto fct = FunctionSignature::new_instance<bool()>::with_id("fct");
    using Args = std::tuple<>; // create arg tuple
    fct->push_args<Args>(); // push those args to signature

    EXPECT_EQ(ref->match( fct ), true);
    EXPECT_EQ(fct->get_arg_count(), 0);
}

TEST( Function_Signature, push_args_template_1)
{
    auto ref = FunctionSignature::new_instance<bool(double, double)>::with_id("fct");

    auto fct = FunctionSignature::new_instance<bool()>::with_id("fct");
    fct->push_args< std::tuple<double, double> >();

    EXPECT_EQ(ref->match( fct ), true);
    EXPECT_EQ(fct->get_arg_count(), 2);
}


TEST( Function_Signature, push_args_template_4)
{
    auto ref = FunctionSignature::new_instance<bool(double, double, double, double)>::with_id("fct");

    auto fct = FunctionSignature::new_instance<bool()>::with_id("fct");
    fct->push_args< std::tuple<double, double, double, double> >();

    EXPECT_EQ(ref->match( fct ), true);
    EXPECT_EQ(fct->get_arg_count(), 4);
}
