#include <gtest/gtest.h>
#include <nodable/core/Log.h>
#include "../test_tools.h"

using namespace Nodable;


TEST(Parser, Atomic_expressions)
{
    EXPECT_EQ(ParseAndEvalExpression<double>("5.0"), 5.0);
    EXPECT_EQ(ParseAndEvalExpression<double>("10.0"), 10.0);
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
    EXPECT_EQ(ParseAndEvalExpression<double>("-1.0+2.0*5.0-3.0/6.0"), 8.5);
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
    EXPECT_EQ(ParseAndEvalExpression<double>("(2.0 -(5.0+3.0 )-2.0)+9.0/(1.0- 0.54)"), (2.0 - (5.0 + 3.0) - 2.0) + 9.0 / (1.0 - 0.54));
    EXPECT_EQ(ParseAndEvalExpression<double>("1.0/3.0"), 1.0 / 3.0);
}

TEST(Parser, unexisting_function)
{
    const std::string code{"pow_unexisting(5)"};
    EXPECT_ANY_THROW(ParseAndEvalExpression<double>(code) );
}

TEST(Parser, function_call)
{
    EXPECT_EQ(ParseAndEvalExpression<int>("return(5)"), 5);
    EXPECT_EQ(ParseAndEvalExpression<int>("sqrt(81)"), 9);
    EXPECT_EQ(ParseAndEvalExpression<int>("pow(2,2)"), 4);
}

TEST(Parser, functionlike_operator_call)
{
    EXPECT_EQ(ParseAndEvalExpression<int>("operator*(2,2)"), 4);
    EXPECT_EQ(ParseAndEvalExpression<bool>("operator>(2,2)"), false);
    EXPECT_EQ(ParseAndEvalExpression<int>("operator-(3,2)"), 1);
    EXPECT_EQ(ParseAndEvalExpression<int>("operator+(2,2)"), 4);
    EXPECT_EQ(ParseAndEvalExpression<int>("operator/(4,2)"), 2);
}

TEST(Parser, imbricated_functions)
{
    EXPECT_EQ(ParseAndEvalExpression<int>("return(5+3)"), 8);
    EXPECT_EQ(ParseAndEvalExpression<int>("return(return(1))"), 1);
    EXPECT_EQ(ParseAndEvalExpression<int>("return(return(1) + return(1))"), 2);
}

TEST(Parser, Successive_assigns)
{
    EXPECT_EQ(ParseAndEvalExpression<double>("double a; double b; a = b = 5.0;"), 5.0);
}

TEST(Parser, Strings)
{
    EXPECT_EQ(ParseAndEvalExpression<std::string>("string a = \"coucou\"")       , "coucou");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("string a = to_string(15)")    , "15");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("string a = to_string(-15)")   , "-15");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("string a = to_string(-15.5)") , "-15.5");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("string b = to_string(true)")  , "true");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("string b = to_string(false)") , "false");
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
           // "double a=5",
            //"double a=1;double b=2;double c=3;double d=4;(a+b)*(c+d)",
            //"string b = to_string(false)"
    };
    ParseEvalSerializeExpressions(expressions);
}

TEST(Parser, Declare_and_define_string)
{
    std::string program = R"(string s = "coucou";)";
    EXPECT_EQ( ParseAndSerialize(program), program);
}

TEST(Parser, Declare_and_define_double)
{
    std::string program = "double d = 15.0;";
    EXPECT_EQ( ParseAndSerialize(program), program);
}

TEST(Parser, Declare_and_define_int)
{
    std::string program = "int s = 10;";
    EXPECT_EQ( ParseAndSerialize(program), program);
}

TEST(Parser, Declare_and_define_bool)
{
    std::string program = "bool b = true;";
    EXPECT_EQ( ParseAndSerialize(program), program);
}

TEST(Parser, Multiple_Instructions_Single_Line )
{
    EXPECT_EQ(ParseAndEvalExpression<int>("int a = 5;int b = 2 * 5;"), 10 );
}

TEST(Parser, Multiple_Instructions_Multi_Line )
{
    EXPECT_EQ(ParseAndEvalExpression<double>("double a = 5.0;\ndouble b = 2.0 * a;"), 10.0 );

    std::vector<std::string> expressions {
        "double a = 5.0;\ndouble b = 2.0 * a;",
        "double a = 5.0;double b = 2.0 * a;\ndouble c = 33.0 + 5.0;"
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
            "double alice = 11;"
            "string message;"
            "if(bob<alice){"
            "   message= \"Alice is the best.\";"
            "}else{"
            "   message= \"Alice is not the best.\";"
            "}";

    ParseAndSerialize({program});
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

    ParseAndSerialize({program});
}

TEST(Parser, not_equals)
{
    EXPECT_TRUE( ParseAndEvalExpression<bool>("10.0 != 9.0;") );
    EXPECT_FALSE( ParseAndEvalExpression<bool>("10.0 != 10.0;") );
}

TEST(Parser, undeclared_variables)
{
    std::string program1 = "double a = b + c * r - z;";
    EXPECT_EQ( ParseAndSerialize(program1), program1);

    std::string program2 = "if(a==b){}";
    EXPECT_EQ( ParseAndSerialize(program2), program2);
}

TEST(Parser, not)
{
    EXPECT_TRUE( ParseAndEvalExpression<bool>("!false;") );
}


TEST(Parser, ignored_chars_pre_ribbon )
{
    std::vector<std::string> expressions {
            " double a = 5"
    };
    ParseEvalSerializeExpressions(expressions);
}

TEST(Parser, ignored_chars_post_ribbon )
{
    std::vector<std::string> expressions {
            "double a = 5 "
    };
    ParseEvalSerializeExpressions(expressions);
}

TEST(Parser, empty_scope )
{
    EXPECT_EQ( ParseAndSerialize("{}"), "{}");
}

TEST(Parser, empty_scope_with_spaces )
{
    EXPECT_EQ( ParseAndSerialize("{ }"), "{ }");
    EXPECT_EQ( ParseAndSerialize("{} "), "{} ");
    EXPECT_EQ( ParseAndSerialize(" {}"), " {}");
    EXPECT_EQ( ParseAndSerialize(" {} "), " {} ");
}

TEST(Parser, empty_program )
{
    EXPECT_EQ( ParseAndSerialize(""), "");
    EXPECT_EQ( ParseAndSerialize(" "), " ");
}