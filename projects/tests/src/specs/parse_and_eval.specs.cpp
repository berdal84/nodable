#include "../fixtures/core.h"
#include <gtest/gtest.h>
#include "fw/core/log.h"

using namespace ndbl;

typedef ::testing::Core Parse_and_eval;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, atomic_expression_int)
{
    EXPECT_EQ(eval<int>("5"), 5);
}

TEST_F(Parse_and_eval, atomic_expression_double)
{
    EXPECT_EQ(eval<double>("10.0"), 10.0);
}

TEST_F(Parse_and_eval, atomic_expression_string)
{
    EXPECT_EQ(eval<std::string>("\"hello world!\""), "hello world!");
}

TEST_F(Parse_and_eval, atomic_expression_bool)
{
    EXPECT_EQ(eval<bool>("true"), true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, unary_operators_minus_int)
{
    EXPECT_EQ(eval<int>("-5"), -5);
}

TEST_F(Parse_and_eval, unary_operators_minus_double)
{
    EXPECT_EQ(eval<double>("-5.5"), -5.5);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, Simple_binary_expressions)
{
    EXPECT_EQ(eval<int>("2+3"), 5);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, Precedence_one_level)
{
    EXPECT_EQ(eval<int>("-5+4"), -1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, Precedence_two_levels)
{
    EXPECT_EQ(eval<double>("-1.0+2.0*5.0-3.0/6.0"), 8.5);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, parenthesis_01)
{
    EXPECT_EQ(eval<int>("(1+4)"), 5);
}

TEST_F(Parse_and_eval, parenthesis_02)
{
    EXPECT_EQ(eval<int>("(1)+(2)"), 3);
}

TEST_F(Parse_and_eval, parenthesis_03)
{
    EXPECT_EQ(eval<int>("(1+2)*3"), 9);
}

TEST_F(Parse_and_eval, parenthesis_04)
{
    EXPECT_EQ(eval<int>("2*(5+3)"), 16);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, unary_binary_operator_mixed_01)
{
    EXPECT_EQ(eval<int>("-1*20"), -20);
}

TEST_F(Parse_and_eval, unary_binary_operator_mixed_02)
{
    EXPECT_EQ(eval<int>("-(1+4)"), -5);
}

TEST_F(Parse_and_eval, unary_binary_operator_mixed_03)
{
    EXPECT_EQ(eval<int>("(-1)+(-2)"), -3);
}
TEST_F(Parse_and_eval, unary_binary_operator_mixed_04)
{
    EXPECT_EQ(eval<int>("-5*3"), -15);
}

TEST_F(Parse_and_eval, unary_binary_operator_mixed_05)
{
    EXPECT_EQ(eval<int>("2-(5+3)"), -6);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, complex_parenthesis_01)
{
    EXPECT_EQ(eval<int>("2+(5*3)"), 2 + (5 * 3));
}

TEST_F(Parse_and_eval, complex_parenthesis_02)
{
    EXPECT_EQ(eval<int>("2*(5+3)+2"), 2 * (5 + 3) + 2);
}

TEST_F(Parse_and_eval, complex_parenthesis_03)
{
    EXPECT_EQ(eval<int>("(2-(5+3))-2+(1+1)"), (2 - (5 + 3)) - 2 + (1 + 1));
}

TEST_F(Parse_and_eval, complex_parenthesis_04)
{
    EXPECT_EQ(eval<double>("(2.0 -(5.0+3.0 )-2.0)+9.0/(1.0- 0.54)"), (2.0 - (5.0 + 3.0) - 2.0) + 9.0 / (1.0 - 0.54));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, unexisting_function)
{
    EXPECT_ANY_THROW(eval<double>("pow_unexisting(5)") );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, function_call_return)
{
    EXPECT_EQ(eval<int>("return(5)"), 5);
}

TEST_F(Parse_and_eval, function_call_sqrt_int)
{
    EXPECT_EQ(eval<int>("sqrt(81)"), 9);
}

TEST_F(Parse_and_eval, function_call_pow_int_int)
{
    EXPECT_EQ(eval<int>("pow(2,2)"), 4);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, functionlike_operator_call_multiply)
{
    EXPECT_EQ(eval<int>("operator*(2,2)"), 4);
}

TEST_F(Parse_and_eval, functionlike_operator_call_superior)
{
    EXPECT_EQ(eval<bool>("operator>(2,2)"), false);
}

TEST_F(Parse_and_eval, functionlike_operator_call_minus)
{
    EXPECT_EQ(eval<int>("operator-(3,2)"), 1);
}

TEST_F(Parse_and_eval, functionlike_operator_call_plus)
{
    EXPECT_EQ(eval<int>("operator+(2,2)"), 4);
}

TEST_F(Parse_and_eval, functionlike_operator_call_divide)
{
    EXPECT_EQ(eval<int>("operator/(4,2)"), 2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, imbricated_operator_in_function)
{
    EXPECT_EQ(eval<int>("return(5+3)"), 8);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, imbricated_functions_01)
{
    EXPECT_EQ(eval<int>("return(return(1))"), 1);
}

TEST_F(Parse_and_eval, imbricated_functions_02)
{
    EXPECT_EQ(eval<int>("return(return(1) + return(1))"), 2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, Successive_assigns)
{
    EXPECT_EQ(eval<double>("double a; double b; a = b = 5.0;"), 5.0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, string_var_assigned_with_literal_string)
{
    EXPECT_EQ(eval<std::string>("string a = \"coucou\"")       , "coucou");
}

TEST_F(Parse_and_eval, string_var_assigned_with_15_to_string)
{
    EXPECT_EQ(eval<std::string>("string a = to_unquoted_string(15)")    , "15");
}

TEST_F(Parse_and_eval, string_var_assigned_with_minus15dot5_to_string)
{
    EXPECT_EQ(eval<std::string>("string a = to_unquoted_string(15.5)")    , "15.5");
}

TEST_F(Parse_and_eval, string_var_assigned_with_true_to_string)
{
    EXPECT_EQ(eval<std::string>("string a = to_unquoted_string(true)")    , "true");
}

TEST_F(Parse_and_eval, string_var_assigned_with_false_to_string)
{
    EXPECT_EQ(eval<std::string>("string a = to_unquoted_string(false)")    , "false");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, precedence_1)
{
    const std::string source_code = "(1+1)*2";
    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

TEST_F(Parse_and_eval, precedence_2)
{
    const std::string source_code = "1*1+2";
    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

TEST_F(Parse_and_eval, precedence_3)
{
    const std::string source_code = "-(-1)";
    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

TEST_F(Parse_and_eval, precedence_4)
{
    const std::string source_code = "-(2*5)";
                    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

TEST_F(Parse_and_eval, precedence_5)
{
    const std::string source_code = "-(2*5)";
                    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

TEST_F(Parse_and_eval, precedence_6)
{
    const std::string source_code = "(-2)*5";
                    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

TEST_F(Parse_and_eval, precedence_7)
{
    const std::string source_code = "-(2+5)";
                    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

TEST_F(Parse_and_eval, precedence_8)
{
    const std::string source_code = "5+(-1)*3";
    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, eval_serialize_and_compare_1)
{
    const std::string source_code = "1";
    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

TEST_F(Parse_and_eval, eval_serialize_and_compare_2)
{
    const std::string source_code = "1+1";
    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

TEST_F(Parse_and_eval, eval_serialize_and_compare_3)
{
    const std::string source_code = "1-1";
    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

TEST_F(Parse_and_eval, eval_serialize_and_compare_4)
{
    const std::string source_code = "-1";
    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

TEST_F(Parse_and_eval, eval_serialize_and_compare_5)
{
    const std::string source_code = "double a=5";
    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

TEST_F(Parse_and_eval, eval_serialize_and_compare_6)
{
    const std::string source_code = "double a=1;double b=2;double c=3;double d=4;(a+b)*(c+d)";
    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

TEST_F(Parse_and_eval, eval_serialize_and_compare_7)
{
    const std::string source_code = "string b = to_unquoted_string(false)";
    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, decl_var_and_assign_string)
{
    std::string program = R"(string s = "coucou";)";
    EXPECT_EQ(parse_and_serialize(program), program);
}

TEST_F(Parse_and_eval, decl_var_and_assign_double)
{
    std::string program = "double d = 15.0;";
    EXPECT_EQ(parse_and_serialize(program), program);
}

TEST_F(Parse_and_eval, decl_var_and_assign_int)
{
    std::string program = "int s = 10;";
    EXPECT_EQ(parse_and_serialize(program), program);
}

TEST_F(Parse_and_eval, decl_var_and_assign_bool)
{
    std::string program = "bool b = true;";
    EXPECT_EQ(parse_and_serialize(program), program);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, multi_instruction_single_line )
{
    EXPECT_EQ(eval<int>("int a = 5;int b = 2 * 5;"), 10 );
}

TEST_F(Parse_and_eval, multi_instruction_multi_line_01 )
{
    const std::string source_code = "double a = 5.0;\ndouble b = 2.0 * a;";
    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

TEST_F(Parse_and_eval, multi_instruction_multi_line_02 )
{
    const std::string source_code = "double a = 5.0;double b = 2.0 * a;\ndouble c = 33.0 + 5.0;";
    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, dna_to_protein )
{
    EXPECT_EQ(eval<std::string>("dna_to_protein(\"TAA\")"), "_");
    EXPECT_EQ(eval<std::string>("dna_to_protein(\"TAG\")"), "_");
    EXPECT_EQ(eval<std::string>("dna_to_protein(\"TGA\")"), "_");
    EXPECT_EQ(eval<std::string>("dna_to_protein(\"ATG\")"), "M");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, code_formatting_preserving_01 )
{
    const std::string source_code = "double a =5;\ndouble b=2*a;";
    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

TEST_F(Parse_and_eval, code_formatting_preserving_02 )
{
    const std::string source_code = "double a =5;\ndouble b=2  *  a;";
    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

TEST_F(Parse_and_eval, code_formatting_preserving_03 )
{
    const std::string source_code = " 5 + 2;";
    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

TEST_F(Parse_and_eval, code_formatting_preserving_04 )
{
    const std::string source_code = "5 + 2;  ";
    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, Conditional_Structures_IF )
{
    std::string program =
            "double bob   = 10;"
            "double alice = 10;"
            "if(bob>alice){"
            "   string message = \"Bob is better than Alice.\";"
            "}";

    EXPECT_EQ( parse_and_serialize(program), program);
}

TEST_F(Parse_and_eval, Conditional_Structures_IF_ELSE )
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

    EXPECT_EQ( parse_and_serialize(program), program);
}

TEST_F(Parse_and_eval, Conditional_Structures_IF_ELSE_IF )
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

    EXPECT_EQ( parse_and_serialize(program), program);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, operator_not_equal_double_double_true)
{
    EXPECT_TRUE(eval<bool>("10.0 != 9.0;") );
}

TEST_F(Parse_and_eval, operator_not_equal_double_double_false)
{
    EXPECT_FALSE(eval<bool>("10.0 != 10.0;") );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, operator_not_bool)
{
    EXPECT_TRUE(eval<bool>("!false;") );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, parse_serialize_with_undeclared_variables )
{
    const std::string program = "double a = b + c * r - z;";
    EXPECT_EQ(parse_and_serialize(program), program);
}

TEST_F(Parse_and_eval, parse_serialize_with_undeclared_variables_in_conditional )
{
    const std::string program = "if(a==b){}";
    EXPECT_EQ(parse_and_serialize(program), program);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, parse_serialize_with_pre_ribbon_chars )
{
    const std::string source_code = " double a = 5";
    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

TEST_F(Parse_and_eval, parse_serialize_with_post_ribbon_chars )
{
    const std::string source_code = "double a = 5 ";
    EXPECT_EQ(parse_eval_and_serialize(source_code), source_code);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Parse_and_eval, parse_serialize_empty_scope )
{
    EXPECT_EQ(parse_and_serialize("{}"), "{}");
}

TEST_F(Parse_and_eval, parse_serialize_empty_scope_with_spaces )
{
    EXPECT_EQ(parse_and_serialize("{ }"), "{ }");
}

TEST_F(Parse_and_eval, parse_serialize_empty_scope_with_spaces_after )
{
    EXPECT_EQ(parse_and_serialize("{} "), "{} ");
}

TEST_F(Parse_and_eval, parse_serialize_empty_scope_with_spaces_before )
{
    EXPECT_EQ(parse_and_serialize(" {}"), " {}");
}

TEST_F(Parse_and_eval, parse_serialize_empty_scope_with_spaces_before_and_after )
{
    EXPECT_EQ(parse_and_serialize(" {} "), " {} ");
}

TEST_F(Parse_and_eval, parse_serialize_empty_program )
{
    EXPECT_EQ(parse_and_serialize(""), "");
}

TEST_F(Parse_and_eval, parse_serialize_empty_program_with_space )
{
    EXPECT_EQ(parse_and_serialize(" "), " ");
}

TEST_F(Parse_and_eval, parse_serialize_single_line_program_with_a_comment_before )
{
    fw::log::set_verbosity("Parser", fw::log::Verbosity_Verbose);
    std::string program =
            "// comment\n"
            "int a = 42;";
    EXPECT_EQ(parse_and_serialize(program), program);
}

TEST_F(Parse_and_eval, parse_serialize_single_program_line_with_two_sigle_line_comments_and_a_space )
{
    std::string program =
            "// first line\n"
            "// second line\n"
            "\n"
            "int a = 42;";
    EXPECT_EQ(parse_and_serialize(program), program);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
