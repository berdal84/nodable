#include <gtest/gtest.h>
#include <cmath> // for pow()

#include <nodable/core/Log.h>
#include "../test_tools.h"

using namespace Nodable;

TEST(VM, Cond_1 )
{
    std::string program =
            "int bob   = 50;"
            "int alice = 10;"
            "bool val;"
            "if(bob>alice)"
            "{"
            "   val = true;"
            "}"
            "else"
            "{"
            "   val = false;"
            "}"
            "return(val);";

    EXPECT_EQ( ParseAndEvalExpression<bool>(program), true );
}

TEST(VM, Cond_2 )
{
    std::string program =
            "int bob   = 0;"
            "int alice = 10;"
            "string str = \"default\";"
            "if(bob>alice)"
            "{"
            "   str = \"true\";"
            "}"
            "else if (false)"
            "{"
            "   str = \"false\";"
            "}"
            "return(str);";

    EXPECT_EQ( ParseAndEvalExpression<std::string>(program), "default" );
}

TEST(VM, Loop_1_using_global_var )
{
    std::string program =
            "string str = \"\";" \
            "for(int n=0;n<10;n=n+1)"
            "{"
            "   str = str + to_string(n);"
            "}"
            "return(str);";

    EXPECT_EQ( ParseAndEvalExpression<std::string>(program), "0123456789" );
}

TEST(VM, Loop_1_using_local_var )
{
    std::string program =
            "string str = \"\";"
            "for(int n=0; n<10; n=n+1)"
            "{"
            "   string tmp = to_string(n);"
            "   str = str + tmp;"
            "}"
            "return(str);";

    EXPECT_EQ( ParseAndEvalExpression<std::string>(program), "0123456789" );
}

TEST(VM, Loop_2_using_global_var )
{
    std::string program =
            "int n;"
            "int p;"
            "string str = \"\";"
            "for(n=0; n<10; n=n+1)"
            "{"
            "   p = pow(n,2);"
            "   if( p != n )/* skip powers equals to n */"
            "   {"
            "      str = str + to_string(p);     /* concat powers */"
            "   }"
            "   else"
            "   {"
            "      str = str + \"_\"; /* concat \"_\" */"
            "   }"
            "}"
            "return(str);";


    EXPECT_EQ( ParseAndEvalExpression<std::string>(program), "__49162536496481" );
}

TEST(VM, Loop_2_using_local_var )
{
    std::string program =
            "string str = \"\";"
            "for(int n=0; n<10; n=n+1)"
            "{"
            "   int p = pow(n,2);"
            "   if( p != n ) /* skip powers equals to n */"
            "   {"
            "      str = str + to_string(p);     /* concat powers */"
            "   }"
            "   else"
            "   {"
            "      str = str + \"_\"; /* concat \"_\" */"
            "   }"
            "}"
            "return(str);";

    EXPECT_EQ( ParseAndEvalExpression<std::string>(program), "__49162536496481" );
}

TEST(VM, For_loop_without_var_decl)
{
std::string program =
        "int score;"
        "for(int i=0; i<10; i=i+1)"
        "{"
        "   score = i * 2;"
        "}"
        "return(score);";
EXPECT_EQ(ParseAndEvalExpression<int>(program), 9*2);
}

TEST(VM, For_loop_with_var_decl)
{
    std::string program =
            "int score = 1;"
            "for(int i=0; i<10; i=i+1 )"
            "{"
            "   score = score * 2;"
            "}"
            "return(score);";
    EXPECT_EQ(ParseAndEvalExpression<int>(program), 1 * pow(2, 10) );
}

TEST(VM, declare_then_define ) {
    std::string program_01 =
            "int b;"
            "b = 5;"
            "return(b);";
    EXPECT_EQ(ParseAndEvalExpression<int>(program_01), 5);
}

TEST(VM, declare_and_define_then_reassign ) {
    std::string program_01 =
            "int b = 6;"
            "b = 5;"
            "return(b);";
    EXPECT_EQ(ParseAndEvalExpression<int>(program_01), 5);
}

TEST(VM, declare_then_define_then_reassign ) {
    std::string program_01 =
            "int b;"
            "b = 6;"
            "b = 5;"
            "return(b);";
    EXPECT_EQ(ParseAndEvalExpression<int>(program_01), 5);
}

TEST(VM, condition_which_contains_alterated_var ) {
    std::string program =
            "int b = 6;"
            "b = 5;"
            "string res = \"ok\";"
            "if( b==6 )"
            "{"
            "  res=\"error\";"
            "}"
            "return(res);";
    EXPECT_EQ( ParseAndEvalExpression<std::string>(program), "ok");
}

TEST(VM, else_elseif_else ) {

    std::string program_end =
            "string msg;\n"
            "if ( a > b ) {\n"
            "    msg = \"a > b\";\n"
            "} else if ( a < b ) {\n"
            "    msg = \"a < b\";\n"
            "} else {\n"
            "    msg = \"a == b\";\n"
            "}"
            "return(msg)";

    std::string program1 = "double a = 6;\ndouble b = 5;\n" + program_end;
    EXPECT_EQ( ParseAndEvalExpression<std::string>(program1), "a > b");

    std::string program2 = "double a = 4;\ndouble b = 5;\n" + program_end;
    EXPECT_EQ( ParseAndEvalExpression<std::string>(program2), "a < b");

    std::string program3 = "double a = 5;\ndouble b = 5;\n" + program_end;
    EXPECT_EQ( ParseAndEvalExpression<std::string>(program3), "a == b");
}

TEST(VM, integers ) {
    EXPECT_EQ(ParseAndEvalExpression<int>("int i = 1"), 1);
    EXPECT_EQ(ParseAndEvalExpression<int>("int i = 3 + 5"), 8);
    EXPECT_EQ(ParseAndEvalExpression<int>("int i = 1-2"), -1);
}

