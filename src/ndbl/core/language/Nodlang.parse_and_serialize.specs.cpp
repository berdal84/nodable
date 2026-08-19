#include "../fixtures/core.h"
#include <gtest/gtest.h>
#include "tools/core/Log.h"

using namespace ndbl;
using namespace tools;

typedef ::testing::Core Language_parse_and_serialize;
typedef ::testing::Core DISABLED_Language_parse_and_serialize;


TEST_F(Language_parse_and_serialize, decl_var_and_assign_string)
{
    bdc::String program = R"(string s = "coucou";)";
    EXPECT_EQ(parse_and_serialize(program), program);
}

TEST_F(Language_parse_and_serialize, decl_var_and_assign_double)
{
    bdc::String program = "double d = 15.0;";
    EXPECT_EQ(parse_and_serialize(program), program);
}

TEST_F(Language_parse_and_serialize, decl_var_and_assign_int)
{
    bdc::String program = "int s = 10;";
    EXPECT_EQ(parse_and_serialize(program), program);
}

TEST_F(Language_parse_and_serialize, decl_var_and_assign_bool)
{
    bdc::String program = "bool b = true;";
    EXPECT_EQ(parse_and_serialize(program), program);
}

/////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, While_loop )
{
    bdc::String program =
            "int i = 0;"
            "while(i < 10){"
            "   i = i+1;"
            "}";

    EXPECT_EQ( parse_and_serialize(program), program);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, Conditional_Structures_IF )
{
    bdc::String program =
            "double bob   = 10;"
            "double alice = 10;"
            "if(bob>alice){"
            "   string message = \"Bob is better than Alice.\";"
            "}";

    EXPECT_EQ( parse_and_serialize(program), program);
}

TEST_F(Language_parse_and_serialize, Conditional_Structures_IF_ELSE )
{
    bdc::String program =
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

TEST_F(Language_parse_and_serialize, Conditional_Structures_IF_ELSE_IF )
{
    bdc::String program =
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

    EXPECT_EQ( parse_and_serialize(program), program);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, parse_serialize_with_undeclared_variables )
{
    const bdc::String program = "double a = b + c * r - z;";
    EXPECT_EQ(parse_and_serialize(program), program);
}

TEST_F(Language_parse_and_serialize, parse_serialize_with_undeclared_variables_in_conditional )
{
    const bdc::String program = "if(a==b){}";
    EXPECT_EQ(parse_and_serialize(program), program);
}

/////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, parse_serialize_empty_program )
{
    EXPECT_EQ(parse_and_serialize(""), "");
}

TEST_F(Language_parse_and_serialize, parse_serialize_empty_program_with_space )
{
    EXPECT_EQ(parse_and_serialize(" "), " ");
}

TEST_F(Language_parse_and_serialize, parse_serialize_single_line_program_with_a_comment_before )
{
    bdc::String program =
            "// comment\n"
            "int a = 42;";
    EXPECT_EQ(parse_and_serialize(program), program);
}

TEST_F(Language_parse_and_serialize, parse_serialize_single_program_line_with_two_sigle_line_comments_and_a_space )
{
    bdc::String program =
            "// first line\n"
            "// second line\n"
            "\n"
            "int a = 42;";
    EXPECT_EQ(parse_and_serialize(program), program);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, parse_serialize_binary_expression_with_funtion )
{
    bdc::String program = "int i = pow(2,2) + 1";
    EXPECT_EQ(parse_and_serialize(program), program); // should not be "int i = (pow(2,2))+ 1"
}

/////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, parse_serialize_variable_declaration )
{
    const bdc::String code = "int i = 42;";
    EXPECT_STREQ(parse_and_serialize(code).c_str(), code);
}

/////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, parse_serialize_variable_referenced )
{
    const bdc::String code = "int i = 42; int j = i;";
    EXPECT_STREQ(parse_and_serialize(code).c_str(), code);
}

TEST_F(Language_parse_and_serialize, parse_serialize_variable_referenced2 )
{
    const bdc::String code = "int i = 42; i;";
    EXPECT_STREQ(parse_and_serialize(code).c_str(), code);
}

/////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, parse_serialize_empty_scope )
{
    EXPECT_EQ(parse_and_serialize("{}"), "{}");
}

TEST_F(Language_parse_and_serialize, parse_serialize_empty_scope_with_spaces )
{
    EXPECT_EQ(parse_and_serialize("{ }"), "{ }");
}

TEST_F(Language_parse_and_serialize, parse_serialize_empty_scope_with_spaces_after )
{
    EXPECT_EQ(parse_and_serialize("{} "), "{} ");
}

TEST_F(Language_parse_and_serialize, parse_serialize_empty_scope_with_spaces_before )
{
    EXPECT_EQ(parse_and_serialize(" {}"), " {}");
}

TEST_F(Language_parse_and_serialize, parse_serialize_empty_scope_with_spaces_before_and_after )
{
    EXPECT_EQ(parse_and_serialize(" {} "), " {} ");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, partial_for1)
{
    bdc::String program = "for();";
    EXPECT_EQ(parse_and_serialize(program), program);
}

TEST_F(Language_parse_and_serialize, partial_for2)
{
    bdc::String program = "for(;);";
    EXPECT_EQ(parse_and_serialize(program), program);
}

TEST_F(Language_parse_and_serialize, partial_for3)
{
    bdc::String program = "for(;;);";
    EXPECT_EQ(parse_and_serialize(program), program);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, partial_if1)
{
    bdc::String program = "if();";
    EXPECT_EQ(parse_and_serialize(program), program);
}

TEST_F(Language_parse_and_serialize, partial_if2)
{
    bdc::String program = "if();else;";
    EXPECT_EQ(parse_and_serialize(program), program);
    bdc::String program2 = "if()else;";
    EXPECT_EQ(parse_and_serialize(program2), "");
    bdc::String program3 = "if()else";
    EXPECT_EQ(parse_and_serialize(program3), "");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, partial_while1)
{
    bdc::String program = "while();";
    EXPECT_EQ(parse_and_serialize(program), program);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize , exemple_arithmetic)
{
    bdc::String program = load_file("examples/arithmetic.cpp");
    EXPECT_EQ(parse_and_serialize(program), program);
}

TEST_F(Language_parse_and_serialize , example_for_loop)
{
    bdc::String program = load_file("examples/for-loop.cpp");
    EXPECT_EQ(parse_and_serialize(program), program);
}

// TODO: handle missing spaces around in var refs
TEST_F(DISABLED_Language_parse_and_serialize , example_if_else)
{
    bdc::String program = load_file("examples/if-else.cpp");
    EXPECT_EQ(parse_and_serialize(program), program);
}

// TODO: handle missing spaces around in var refs
TEST_F(DISABLED_Language_parse_and_serialize , exemple_multi_instructions)
{
    bdc::String program = load_file("examples/multi-instructions.cpp");
    EXPECT_EQ(parse_and_serialize(program), program);
}