#include "../fixtures/core.h"
#include <gtest/gtest.h>
#include "tools/core/Log.h"

using namespace ndbl;
using namespace tools;

typedef ::testing::Core Language_parse_and_serialize;
typedef ::testing::Core DISABLED_Language_parse_and_serialize;


TEST_F(Language_parse_and_serialize, decl_var_and_assign_string)
{
    String code   = R"(string s = "coucou";)";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, decl_var_and_assign_double)
{
    String code   = "double d = 15.0;";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, decl_var_and_assign_int)
{
    String code   = "int s = 10;";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, decl_var_and_assign_bool)
{
    String code   = "bool b = true;";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

/////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, While_loop )
{
    String code =
        "int i = 0;"
        "while(i < 10){"
        "   i = i+1;"
        "}";
    String result = parse_and_serialize(code);
    EXPECT_EQ( result, code);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, Conditional_Structures_IF )
{
    String code =
        "double bob   = 10;"
        "double alice = 10;"
        "if(bob>alice){"
        "   string message = \"Bob is better than Alice.\";"
        "}";
    String result = parse_and_serialize(code);
    EXPECT_EQ( result, code);
}

TEST_F(Language_parse_and_serialize, Conditional_Structures_IF_ELSE )
{
    String code =
        "double bob   = 10;"
        "double alice = 11;"
        "string message;"
        "if(bob<alice){"
        "   message= \"Alice is the best.\";"
        "}else{"
        "   message= \"Alice is not the best.\";"
        "}";
    String result = parse_and_serialize(code);
    EXPECT_EQ( result, code);
}

TEST_F(Language_parse_and_serialize, Conditional_Structures_IF_ELSE_IF )
{
    String code =
        "double bob   = 10;"
        "double alice = 10;"
        "string message;"
        "if(bob>alice){"
        "   message= \"Bob is greater than Alice.\";"
        "} else if(bob<alice){"
        "   message= \"Bob is lower than Alice.\";"
        "} else {"
        "   message= \"Bob and Alice are is.\";"
        "}";
    String result = parse_and_serialize(code);
    EXPECT_EQ( result, code);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, parse_serialize_with_undeclared_variables )
{
    String code   = "double a = b + c * r - z;";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, parse_serialize_with_undeclared_variables_in_conditional )
{
    String code   = "if(a==b){}";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

/////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, parse_serialize_empty_code )
{
    String result = parse_and_serialize("");
    EXPECT_EQ( result, "" );
}

TEST_F(Language_parse_and_serialize, parse_serialize_empty_code_with_space )
{
    String result = parse_and_serialize(" ");
    EXPECT_EQ( result, " " );
}

TEST_F(Language_parse_and_serialize, parse_serialize_single_line_code_with_a_comment_before )
{
    String code =
        "// comment\n"
        "int a = 42;";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, parse_serialize_single_code_line_with_two_sigle_line_comments_and_a_space )
{
    String code =
        "// first line\n"
        "// second line\n"
        "\n"
        "int a = 42;";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, parse_serialize_binary_expression_with_funtion )
{
    String code   = "int i = pow(2,2) + 1";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code); // should not be "int i = (pow(2,2))+ 1"
}

/////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, parse_serialize_variable_declaration )
{
    String code   = "int i = 42;";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

/////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, parse_serialize_variable_referenced )
{
    String code   = "int i = 42; int j = i;";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, parse_serialize_variable_referenced2 )
{
    String code   = "int i = 42; i;";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

/////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, parse_serialize_empty_scope )
{
    String result = parse_and_serialize("{}");
    EXPECT_EQ(result, "{}");
}

TEST_F(Language_parse_and_serialize, parse_serialize_empty_scope_with_spaces )
{
    String result = parse_and_serialize("{ }");
    EXPECT_EQ(result, "{ }");
}

TEST_F(Language_parse_and_serialize, parse_serialize_empty_scope_with_spaces_after )
{
    String result = parse_and_serialize("{} ");
    EXPECT_EQ(result, "{} ");
}

TEST_F(Language_parse_and_serialize, parse_serialize_empty_scope_with_spaces_before )
{
    String result = parse_and_serialize(" {}");
    EXPECT_EQ(result, " {}");
}

TEST_F(Language_parse_and_serialize, parse_serialize_empty_scope_with_spaces_before_and_after )
{
    String result = parse_and_serialize(" {} ");
    EXPECT_EQ(result, " {} ");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, partial_for1)
{
    String code = "for();";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, partial_for2)
{
    String code   = "for(;);";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, partial_for3)
{
    String code   = "for(;;);";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, partial_if1)
{
    String code    = "if();";
    String result  = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, partial_if2)
{
    String code    = "if();else;";
    String result  = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, partial_if3)
{
    String code    = "if()else;";
    String result  = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, partial_if4)
{
    String code    = "if()else";
    String result  = parse_and_serialize(code);
    EXPECT_EQ(result, "");
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, partial_while1)
{
    String code   = "while();";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize , exemple_arithmetic)
{
    String code   = load_file("examples/arithmetic.cpp");
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize , example_for_loop)
{
    String code   = load_file("examples/for-loop.cpp");
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

// TODO: handle missing spaces around in var refs
TEST_F(DISABLED_Language_parse_and_serialize , example_if_else)
{
    String code   = load_file("examples/if-else.cpp");
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

// TODO: handle missing spaces around in var refs
TEST_F(DISABLED_Language_parse_and_serialize , exemple_multi_instructions)
{
    String code   = load_file("examples/multi-instructions.cpp");
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}