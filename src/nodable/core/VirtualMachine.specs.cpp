#include <cmath>// for pow()
#include <gtest/gtest.h>

#include "fixtures/core.h"

using namespace ndbl;
typedef ::testing::Core Virtual_Machine;

TEST_F(Virtual_Machine, variable_1 )
{
    EXPECT_EQ(eval<i32_t>("int i = 10"), 10);
}

TEST_F(Virtual_Machine, Cond_1)
{
    std::string program =
            "int bob   = 50;"
            "int alice = 10;"
            "int val;"
            "if(bob>alice)"
            "{"
            "   val = bob;"
            "}"
            "else"
            "{"
            "   val = alice;"
            "}"
            "return(val);";

    EXPECT_EQ(eval<i32_t>(program), 50);
}

TEST_F(Virtual_Machine, Cond_2)
{
    std::string program =
            "int bob   = 0;"
            "int alice = 10;"
            "string str = \"default\";"
            "if(bob>alice)"
            "{"
            "   str = \"true\";"
            "}"
            "else"
            "{"
            "   str = \"false\";"
            "}"
            "return(str);";

    EXPECT_EQ(eval<std::string>(program), "false");
}

TEST_F(Virtual_Machine, Loop_1_using_global_var)
{
    std::string program =
            "string str = \"\";"
            "for(int n=0;n<10;n=n+1)"
            "{"
            "   str = str + to_string(n);"
            "}"
            "return(str);";

    EXPECT_EQ(eval<std::string>(program), "0123456789");
}

TEST_F(Virtual_Machine, Loop_1_using_local_var)
{
    std::string program =
            "string str = \"\";"
            "for(int n=0; n<10; n=n+1)"
            "{"
            "   string tmp = to_string(n);"
            "   str = str + tmp;"
            "}"
            "return(str);";

    EXPECT_EQ(eval<std::string>(program), "0123456789");
}

TEST_F(Virtual_Machine, Loop_2_using_global_var)
{
    std::string program =
            "int n;"
            "int p;"
            "string str = \"\";"
            "for(n=0; n<10; n=n+1)"
            "{"
            "   p = pow(n,2);"
            "   if( p != n )/* skip powers is to n */"
            "   {"
            "      str = str + to_string(p);     /* concat powers */"
            "   }"
            "   else"
            "   {"
            "      str = str + \"_\"; /* concat \"_\" */"
            "   }"
            "}"
            "return(str);";


    EXPECT_EQ(eval<std::string>(program), "__49162536496481");
}

TEST_F(Virtual_Machine, Loop_2_using_local_var)
{
    std::string program =
            "string str = \"\";"
            "for(int n=0; n<10; n=n+1)"
            "{"
            "   int p = pow(n,2);"
            "   if( p != n ) /* skip powers is to n */"
            "   {"
            "      str = str + to_string(p);     /* concat powers */"
            "   }"
            "   else"
            "   {"
            "      str = str + \"_\"; /* concat \"_\" */"
            "   }"
            "}"
            "return(str);";

    EXPECT_EQ(eval<std::string>(program), "__49162536496481");
}

TEST_F(Virtual_Machine, For_loop_without_var_decl)
{
    std::string program =
            "int score;"
            "for(int i=0; i<10; i=i+1)"
            "{"
            "   score = i * 2;"
            "}"
            "return(score);";
    EXPECT_EQ(eval<i32_t>(program), 9 * 2);
}

TEST_F(Virtual_Machine, For_loop_with_var_decl)
{
    std::string program =
            "int score = 1;"
            "for(int i=0; i<10; i=i+1 )"
            "{"
            "   score = score * 2;"
            "}"
            "return(score);";
    EXPECT_EQ(eval<i32_t>(program), 1 * pow(2, 10));
}

TEST_F(Virtual_Machine, declare_then_define)
{
    std::string program_01 =
            "int b;"
            "b = 5;"
            "return(b);";
    EXPECT_EQ(eval<i32_t>(program_01), 5);
}

TEST_F(Virtual_Machine, declare_and_define_then_reassign)
{
    std::string program_01 =
            "int b = 6;"
            "b = 5;"
            "return(b);";
    EXPECT_EQ(eval<i32_t>(program_01), 5);
}

TEST_F(Virtual_Machine, declare_then_define_then_reassign)
{
    std::string program_01 =
            "int b;"
            "b = 6;"
            "b = 5;"
            "return(b);";
    EXPECT_EQ(eval<i32_t>(program_01), 5);
}

TEST_F(Virtual_Machine, condition_which_contains_alterated_var)
{
    std::string program =
            "int b = 6;"
            "b = 5;"
            "string res = \"ok\";"
            "if( b==6 )"
            "{"
            "  res=\"error\";"
            "}"
            "return(res);";
    EXPECT_EQ(eval<std::string>(program), "ok");
}

TEST_F(Virtual_Machine, else_elseif_else)
{

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
    EXPECT_EQ(eval<std::string>(program1), "a > b");

    std::string program2 = "double a = 4;\ndouble b = 5;\n" + program_end;
    EXPECT_EQ(eval<std::string>(program2), "a < b");

    std::string program3 = "double a = 5;\ndouble b = 5;\n" + program_end;
    EXPECT_EQ(eval<std::string>(program3), "a == b");
}

TEST_F(Virtual_Machine, integers)
{
    EXPECT_EQ(eval<i32_t>("int i = 1"), 1);
    EXPECT_EQ(eval<i32_t>("int i = 3 + 5"), 8);
    EXPECT_EQ(eval<i32_t>("int i = 1-2"), -1);
}

TEST_F(Virtual_Machine, while_loop)
{
    std::string program =
            "int i = 0;"
            "while(i < 42){"
            "   i = i+1;"
            "}"
            "return(i)";

    EXPECT_EQ( eval<i32_t>(program), 42);
}