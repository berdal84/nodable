#include <gtest/gtest.h>

#include <nodable/Function.h>

using namespace Nodable;

TEST( FunctionSignature, no_arg_fct)
{
    FunctionSignature no_arg_fct("no_arg_fct", TokenType_BooleanType);
    EXPECT_EQ(no_arg_fct.getArgCount(), 0);
    EXPECT_EQ(no_arg_fct.getArgCount(), 0);
}

TEST( FunctionSignature, push_single_arg)
{
    FunctionSignature single_arg_fct("single_arg_fct", TokenType_BooleanType);
    single_arg_fct.pushArg(TokenType_DoubleType);
    EXPECT_EQ(single_arg_fct.getArgCount(), 1);
    EXPECT_EQ(single_arg_fct.getType(), TokenType_BooleanType);
    EXPECT_EQ(single_arg_fct.getArgs().at(0).type, TokenType_DoubleType);
}

TEST( FunctionSignature, push_two_args)
{
    FunctionSignature two_arg_fct("two_arg_fct", TokenType_BooleanType);
    two_arg_fct.pushArgs(TokenType_DoubleType, TokenType_DoubleType);
    EXPECT_EQ(two_arg_fct.getArgCount(), 2);
    EXPECT_EQ(two_arg_fct.getType(), TokenType_BooleanType);
    EXPECT_EQ(two_arg_fct.getArgs().at(0).type, TokenType_DoubleType);
    EXPECT_EQ(two_arg_fct.getArgs().at(1).type, TokenType_DoubleType);
}

TEST( FunctionSignature, match_check_for_arg_count)
{
    FunctionSignature single_arg_fct("single_arg_fct", TokenType_BooleanType);
    single_arg_fct.pushArg(TokenType_DoubleType);

    FunctionSignature two_arg_fct("two_arg_fct", TokenType_BooleanType);
    two_arg_fct.pushArgs(TokenType_DoubleType, TokenType_DoubleType);

    EXPECT_EQ(two_arg_fct.match( &single_arg_fct ), false);
    EXPECT_EQ(single_arg_fct.match( &two_arg_fct ), false);
}

TEST( FunctionSignature, match_check_identifier)
{
    FunctionSignature two_arg_fct("two_arg_fct", TokenType_BooleanType);
    two_arg_fct.pushArgs(TokenType_DoubleType, TokenType_DoubleType);

    FunctionSignature two_arg_fct_modified("two_arg_fct_modified", TokenType_BooleanType);
    two_arg_fct_modified.pushArgs(TokenType_DoubleType, TokenType_DoubleType);

    EXPECT_EQ(two_arg_fct.match( &two_arg_fct_modified ), false);
    EXPECT_EQ(two_arg_fct_modified.match( &two_arg_fct ), false);
}

TEST( FunctionSignature, match_check_absence_of_arg)
{
    FunctionSignature two_arg_fct_without_args("two_arg_fct_without_args", TokenType_BooleanType);

    FunctionSignature two_arg_fct("two_arg_fct", TokenType_BooleanType);
    two_arg_fct.pushArgs(TokenType_DoubleType, TokenType_DoubleType);

    EXPECT_EQ(two_arg_fct.match( &two_arg_fct_without_args ), false);
    EXPECT_EQ(two_arg_fct_without_args.match( &two_arg_fct ), false);
}

TEST( FunctionSignature, push_args_template_0)
{
    FunctionSignature ref("ref", TokenType_BooleanType); // reference
    FunctionSignature fct("fct", TokenType_BooleanType); // the one tested

    using Args = std::tuple<>; // create arg tuple
    push_args<Args>(&fct); // push those args to signature

    EXPECT_EQ(ref.match( &fct ), false);
    EXPECT_EQ(fct.getArgCount(), 0);
}

TEST( FunctionSignature, push_args_template_1)
{
    FunctionSignature ref("ref", TokenType_BooleanType); // reference
    ref.pushArgs(TokenType_DoubleType, TokenType_DoubleType);

    FunctionSignature fct("fct", TokenType_BooleanType); // the one tested

    using Args = std::tuple<double, double>; // create arg tuple
    push_args<Args>(&fct); // push those args to signature

    EXPECT_EQ(ref.match( &fct ), false);
    EXPECT_EQ(fct.getArgCount(), 2);
}


TEST( FunctionSignature, push_args_template_2)
{
    FunctionSignature ref("ref", TokenType_BooleanType); // reference
    ref.pushArgs(TokenType_DoubleType, TokenType_DoubleType, TokenType_DoubleType, TokenType_DoubleType);

    FunctionSignature fct("fct", TokenType_BooleanType); // the one tested

    using Args = std::tuple<double, double, double, double>; // create arg tuple
    push_args<Args>(&fct); // push those args to signature

    EXPECT_EQ(ref.match( &fct ), false);
    EXPECT_EQ(fct.getArgCount(), 4);
}
