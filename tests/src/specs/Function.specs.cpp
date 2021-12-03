#include <gtest/gtest.h>

#include <nodable/Function.h>

using namespace Nodable;

TEST( FunctionSignature, no_arg_fct)
{
    FunctionSignature no_arg_fct("no_arg_fct", Type_Boolean);
    EXPECT_EQ(no_arg_fct.getArgCount(), 0);
    EXPECT_EQ(no_arg_fct.getArgCount(), 0);
}

TEST( FunctionSignature, push_single_arg)
{
    FunctionSignature single_arg_fct("single_arg_fct", Type_Boolean);
    single_arg_fct.pushArg(Type_Double);
    EXPECT_EQ(single_arg_fct.getArgCount(), 1);
    EXPECT_EQ(single_arg_fct.getType(), Type_Boolean);
    EXPECT_EQ(single_arg_fct.getArgs().at(0).type, Type_Double);
}

TEST( FunctionSignature, push_two_args)
{
    FunctionSignature two_arg_fct("two_arg_fct", Type_Boolean);
    two_arg_fct.pushArgs(Type_Double, Type_Double);
    EXPECT_EQ(two_arg_fct.getArgCount(), 2);
    EXPECT_EQ(two_arg_fct.getType(), Type_Boolean);
    EXPECT_EQ(two_arg_fct.getArgs().at(0).type, Type_Double);
    EXPECT_EQ(two_arg_fct.getArgs().at(1).type, Type_Double);
}

TEST( FunctionSignature, match_check_for_arg_count)
{
    FunctionSignature single_arg_fct("single_arg_fct", Type_Boolean);
    single_arg_fct.pushArg(Type_Double);

    FunctionSignature two_arg_fct("two_arg_fct", Type_Boolean);
    two_arg_fct.pushArgs(Type_Double, Type_Double);

    EXPECT_EQ(two_arg_fct.match( &single_arg_fct ), false);
    EXPECT_EQ(single_arg_fct.match( &two_arg_fct ), false);
}

TEST( FunctionSignature, match_check_identifier)
{
    FunctionSignature two_arg_fct("two_arg_fct", Type_Boolean);
    two_arg_fct.pushArgs(Type_Double, Type_Double);

    FunctionSignature two_arg_fct_modified("two_arg_fct_modified", Type_Boolean);
    two_arg_fct_modified.pushArgs(Type_Double, Type_Double);

    EXPECT_EQ(two_arg_fct.match( &two_arg_fct_modified ), false);
    EXPECT_EQ(two_arg_fct_modified.match( &two_arg_fct ), false);
}

TEST( FunctionSignature, match_check_absence_of_arg)
{
    FunctionSignature two_arg_fct_without_args("two_arg_fct_without_args", Type_Boolean);

    FunctionSignature two_arg_fct("two_arg_fct", Type_Boolean);
    two_arg_fct.pushArgs(Type_Double, Type_Double);

    EXPECT_EQ(two_arg_fct.match( &two_arg_fct_without_args ), false);
    EXPECT_EQ(two_arg_fct_without_args.match( &two_arg_fct ), false);
}

TEST( FunctionSignature, push_args_template_0)
{
    auto ref = FunctionSignature::Create(Type_Boolean, "ref");

    auto fct = FunctionSignature::Create(Type_Boolean, "fct");
    using Args = std::tuple<>; // create arg tuple
    push_args<Args>(&fct); // push those args to signature

    EXPECT_EQ(ref.match( &fct ), false);
    EXPECT_EQ(fct.getArgCount(), 0);
}

TEST( FunctionSignature, push_args_template_1)
{
    auto ref = FunctionSignature::Create(Type_Boolean, "ref", Type_Double, Type_Double);

    auto fct = FunctionSignature::Create(Type_Boolean, "fct");

    using Args = std::tuple<double, double>; // create arg tuple
    push_args<Args>(&fct); // push those args to signature

    EXPECT_EQ(ref.match( &fct ), false);
    EXPECT_EQ(fct.getArgCount(), 2);
}


TEST( FunctionSignature, push_args_template_2)
{
    auto ref = FunctionSignature::Create(Type_Boolean, "ref", Type_Double, Type_Double, Type_Double, Type_Double);

    auto fct = FunctionSignature::Create(Type_Boolean, "fct");
    using Args = std::tuple<double, double, double, double>; // create arg tuple
    push_args<Args>(&fct); // push those args to signature

    EXPECT_EQ(ref.match( &fct ), false);
    EXPECT_EQ(fct.getArgCount(), 4);
}
