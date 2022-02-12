#include <gtest/gtest.h>
#include <nodable/Log.h>
#include "../tools.h"

using namespace Nodable;


TEST(Parser, Atomic_expressions)
{
    EXPECT_EQ(ParseAndEvalExpression<int>("5"), 5);
    EXPECT_EQ(ParseAndEvalExpression<int>("10"), 10);
}

TEST(Parser, Unary_operators)
{
    EXPECT_EQ(ParseAndEvalExpression<int>("-5"), -5);
}

TEST(Parser, Simple_binary_expressions)
{
    EXPECT_EQ(ParseAndEvalExpression<int>("2+3"), 5);
}

TEST(Parser, Precedence_one_level)
{
    EXPECT_EQ(ParseAndEvalExpression<int>("-5+4"), -1);
}

TEST(Parser, Precedence_two_levels)
{
    EXPECT_EQ(ParseAndEvalExpression<double>("-1+2*5-3/6"), 8.5);
}

TEST(Parser, Simple_parenthesis)
{
    EXPECT_EQ(ParseAndEvalExpression<int>("(1+4)"), 5);
    EXPECT_EQ(ParseAndEvalExpression<int>("(1)+(2)"), 3);
    EXPECT_EQ(ParseAndEvalExpression<int>("(1+2)*3"), 9);
    EXPECT_EQ(ParseAndEvalExpression<int>("2*(5+3)"), 16);
}

TEST(Parser, Unary_Binary_operators_mixed)
{
    EXPECT_EQ(ParseAndEvalExpression<int>("-1*20"), -20);
    EXPECT_EQ(ParseAndEvalExpression<int>("-(1+4)"), -5);
    EXPECT_EQ(ParseAndEvalExpression<int>("(-1)+(-2)"), -3);
    EXPECT_EQ(ParseAndEvalExpression<int>("-5*3"), -15);
    EXPECT_EQ(ParseAndEvalExpression<int>("2-(5+3)"), -6);
}

TEST(Parser, Complex_parenthesis)
{
    EXPECT_EQ(ParseAndEvalExpression<int>("2+(5*3)"), 2 + (5 * 3));
    EXPECT_EQ(ParseAndEvalExpression<int>("2*(5+3)+2"), 2 * (5 + 3) + 2);
    EXPECT_EQ(ParseAndEvalExpression<int>("(2-(5+3))-2+(1+1)"), (2 - (5 + 3)) - 2 + (1 + 1));
    EXPECT_EQ(ParseAndEvalExpression<double>("(2 -(5+3 )-2)+9/(1- 0.54)"), (2 - (5 + 3) - 2) + 9 / (1 - 0.54));
    EXPECT_EQ(ParseAndEvalExpression<double>("1/3"), 1.0 / 3.0);
}

TEST(Parser, unexisting_function)
{
    EXPECT_ANY_THROW(ParseAndEvalExpression<int>("pow_unexisting(5)") );
}

TEST(Parser, function_call)
{
    EXPECT_EQ(ParseAndEvalExpression<int>("returnNumber(5)"), 5);
    EXPECT_EQ(ParseAndEvalExpression<int>("sqrt(81)"), 9);
    EXPECT_EQ(ParseAndEvalExpression<int>("pow(2,2)"), 4);
}

TEST(Parser, functionlike_operator_call)
{
    EXPECT_EQ(ParseAndEvalExpression<double>("operator*(2,2)"), 4.0);
    EXPECT_EQ(ParseAndEvalExpression<bool>("operator>(2,2)"), false);
    EXPECT_EQ(ParseAndEvalExpression<double>("operator-(3,2)"), 1.0);
    EXPECT_EQ(ParseAndEvalExpression<double>("operator+(2,2)"), 4.0);
    EXPECT_EQ(ParseAndEvalExpression<double>("operator/(4,2)"), 2.0);
}

TEST(Parser, imbricated_functions)
{
    EXPECT_EQ(ParseAndEvalExpression<int>("returnNumber(5+3)"), 8);
    EXPECT_EQ(ParseAndEvalExpression<int>("returnNumber(returnNumber(1))"), 1);
    EXPECT_EQ(ParseAndEvalExpression<int>("returnNumber(returnNumber(1) + returnNumber(1))"), 2);
}

TEST(Parser, Successive_assigns)
{
    EXPECT_EQ(ParseAndEvalExpression<double>("double a; double b; a = b = 5;"), 5.0);
}

TEST(Parser, Strings)
{
    EXPECT_EQ(ParseAndEvalExpression<std::string>("string a = \"coucou\""), "coucou");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("string a = to_string(15)"), "15");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("string a = to_string(-15)"), "-15");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("string a = to_string(-15.5)"), "-15.5");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("string b = to_string(true)"), "true");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("string b = to_string(false)"), "false");
}

TEST(Parser, Serialize_Precedence)
{
    std::vector<std::string> expressions
    {
        "(1+1)*2",
        "1*1+2",
        "-(-1)",
        "-(2*5)",
        "(-2)*5",
        "-(2+5)",
        "5+(-1)*3"
    };

    ParseEvalSerializeExpressions(expressions);
}

TEST(Parser, Eval_Serialize_Compare)
{
    std::vector<std::string> expressions
    {
            "1",
            "1+1",
            "1-1",
            "-1",
            "double a = 5",
            "double a=1;double b=2;double c=3;double d=4;(a+b)*(c+d)",
            "string b = to_string(false)"
    };
    Log::SetVerbosityLevel("Runner", Log::Verbosity::Verbose );
    ParseEvalSerializeExpressions(expressions);
}

TEST(Parser, Declare_and_define_vars)
{
    std::vector<std::string> expressions
    {
        "double a = 10.5;",
        R"(string s = "coucou";)",
        "bool b = false;"
    };

    ParseEvalSerializeExpressions(expressions);
}

TEST(Parser, Single_Instruction_With_EndOfInstruction )
{
    EXPECT_EQ(ParseAndEvalExpression<double>("double a = 5;"), 5.0);

    std::vector<std::string> expressions { "double a = 5;" };
    ParseEvalSerializeExpressions(expressions);
}

TEST(Parser, Multiple_Instructions_Single_Line )
{
    EXPECT_EQ(ParseAndEvalExpression<double>("double a = 5;double b = 2 * 5;"), 10.0 );

    std::vector<std::string> expressions { "double a = 5;double b = 2 * 5;" };
    ParseEvalSerializeExpressions(expressions);
}

TEST(Parser, Multiple_Instructions_Multi_Line )
{
    EXPECT_EQ(ParseAndEvalExpression<double>("double a = 5;\ndouble b = 2 * a;"), 10.0 );

    std::vector<std::string> expressions {
        "double a = 5;\ndouble b = 2 * a;",
        "double a = 5;double b = 2 * a;\ndouble c = 33 + 5;"
    };
    ParseEvalSerializeExpressions(expressions);
}

TEST(Parser, DNAtoProtein )
{
    EXPECT_EQ(ParseAndEvalExpression<std::string>("DNAtoProtein(\"TAA\")"), "_");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("DNAtoProtein(\"TAG\")"), "_");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("DNAtoProtein(\"TGA\")"), "_");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("DNAtoProtein(\"ATG\")"), "M");
}

TEST(Parser, Code_Formatting_Preserving )
{
    std::vector<std::string> expressions {
            "double a =5;\ndouble b=2*a;",
            "double a =5;\ndouble b=2  *  a;",
            " 5 + 2;",
            "5 + 2;  "
    };
    ParseEvalSerializeExpressions(expressions);
}

TEST(Parser, Conditional_Structures_IF )
{
    std::string program =
            "double bob   = 10;"
            "double alice = 10;"
            "if(bob>alice){"
            "   string message = \"Bob is better than Alice.\";"
            "}";

    ParseEvalSerializeExpressions({program});
}

TEST(Parser, Conditional_Structures_IF_ELSE )
{
    std::string program =
            "double bob   = 10;"
            "double alice = 10;"
            "string message;"
            "if(bob>alice){"
            "   message= \"Bob is the best.\";"
            "}else if(bob<alice){"
            "   message= \"Alice is the best.\";"
            "}else{"
            "   message= \"Draw.\";"
            "}";

    ParseEvalSerializeExpressions({program});
}

TEST(Parser, Conditional_Structures_IF_ELSE_IF )
{
    std::string program =
            "double bob   = 10;"
            "double alice = 10;"
            "string message;"
            "if(bob>alice){"
            "   message= \"Bob is greater than Alice.\";"
            "} else if(bob<alice){"
            "   message= \"Bob is lower than Alice.\";"
            "} else {"
            "   message= \"Bob and Alice are equals.\";"
            "}";

    ParseEvalSerializeExpressions({program});
}

TEST(Parser, For_loop_without_var_decl)
{
    std::string program =
            "double score;"
            "for(double i=0;i<10;i=i+1){"
            "   score= i*2;"
            "}";
    ParseEvalSerializeExpressions({program});
}

TEST(Parser, For_loop_with_var_decl)
{
    std::string program =
            "double score = 1;"
            "for(double i=0;i<10;i=i+1){"
            "   score= score*2;"
            "}";
    ParseEvalSerializeExpressions({program});
}

TEST(Parser, declare_then_define ) {
    std::string program_01 =
            "double b;"
            "b = 5;"
            "b;";
    EXPECT_EQ(ParseAndEvalExpression<int>(program_01), 5);
}

TEST(Parser, declare_and_define_then_reassign ) {
    std::string program_01 =
            "double b = 6;"
            "b = 5;"
            "b;";
    EXPECT_EQ(ParseAndEvalExpression<int>(program_01), 5);
}

TEST(Parser, declare_then_define_then_reassign ) {
    std::string program_01 =
            "double b;"
            "b = 6;"
            "b = 5;"
            "b;";
    EXPECT_EQ(ParseAndEvalExpression<int>(program_01), 5);
}

TEST(Parser, condition_which_contains_alterated_var ) {
    std::string program_01 =
            "double b = 6;"
            "b = 5;"
            "string res = \"ok\";"
            "if(b==6){"
            "  res=\"error\";"
            "}"
            "res;";
    EXPECT_EQ(ParseAndEvalExpression<std::string>(program_01), "ok");
}


TEST(Parser, not_equals)
{
    EXPECT_TRUE( ParseAndEvalExpression<bool>("10.0 != 9.0;") );
    EXPECT_FALSE( ParseAndEvalExpression<bool>("10.0 != 10.0;") );
}

TEST(Parser, undeclared_variables)
{
    std::string program1 = "double a = b + c;";
    EXPECT_EQ( ParseAndSerialize(program1), program1);

    std::string program2 = "if(a==b){}";
    EXPECT_EQ( ParseAndSerialize(program2), program2);
}

TEST(Parser, not)
{
    EXPECT_TRUE( ParseAndEvalExpression<bool>("!false;") );
}

//TEST(Parser, var_decl_in_cond )
//{
//    std::string program = "if(double b=0){}";
//    ParseEvalSerializeExpressions({program});
//}
