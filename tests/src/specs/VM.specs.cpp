#include <gtest/gtest.h>
#include <cmath> // for pow()

#include <nodable/core/Log.h>
#include "../test_tools.h"

using namespace Nodable;

TEST(VM, Cond_1 )
{
    std::string program =
            "double bob   = 50;"
            "double alice = 10;"
            "bool val;"
            "if(bob>alice)"
            "{"
            "   val = true;"
            "}"
            "else"
            "{"
            "   val = false;"
            "}"
            "val;"; // return-like

    EXPECT_EQ( ParseAndEvalExpression<bool>(program), true );
}

TEST(VM, Cond_2 )
{
    std::string program =
            "double bob   = 0;"
            "double alice = 10;"
            "string val = \"default\";"
            "if(bob>alice)"
            "{"
            "   val = \"true\";"
            "}"
            "else if (false)"
            "{"
            "   val = \"false\";"
            "}"
            "val;"; // return-like

    EXPECT_EQ( ParseAndEvalExpression<std::string>(program), "default" );
}

TEST(VM, Loop_1_using_global_var )
{
    std::string program =
            "string res = \"\";" \
            "for(double n=0;n<10;n=n+1)"
            "{"
            "   res = res + to_string(n);"
            "}"
            "res;";

    EXPECT_EQ( ParseAndEvalExpression<std::string>(program), "0123456789" );
}

TEST(VM, Loop_1_using_local_var )
{
    std::string program =
            "string res = \"\";" \
            "for(double n=0; n<10; n=n+1)"
            "{"
            "   print(\"A: \" + n);"
            "   string tmp = to_string(n);"
            "   print(\"B: \" + n);"
            "   res = res + tmp;"
            "   print(\"C: \" + n);"
            "}"
            "res;";

    EXPECT_EQ( ParseAndEvalExpression<std::string>(program), "0123456789" );
}

TEST(VM, Loop_2_using_global_var )
{
    std::string program =
            "double n;"
            "double p;"
            "string res = \"\";"
            "for(n=0; n<10; n=n+1)"
            "{"
            "   p = pow(n,2);"
            "   if( p != n )/* skip powers equals to n */"
            "   {"
            "      res = res + p;     /* concat powers */"
            "   }"
            "   else"
            "   {"
            "      res = res + \"_\"; /* concat \"_\" */"
            "   }"
            "}"
            "res;";

    EXPECT_EQ( ParseAndEvalExpression<std::string>(program), "__49162536496481" );
}

TEST(VM, Loop_2_using_local_var )
{
    std::string program =
            "string res = \"\";"
            "for(double n=0; n<10; n=n+1)"
            "{"
            "   double p = pow(n,2);"
            "   if( p != n ) /* skip powers equals to n */"
            "   {"
            "      res = res + p;     /* concat powers */"
            "   }"
            "   else"
            "   {"
            "      res = res + \"_\"; /* concat \"_\" */"
            "   }"
            "}"
            "res;";

    EXPECT_EQ( ParseAndEvalExpression<std::string>(program), "__49162536496481" );
}

TEST(VM, For_loop_without_var_decl)
{
std::string program =
        "double score;"
        "for(double i=0; i<10; i=i+1)"
        "{"
        "   score= i*2;"
        "}"
        "score;";
EXPECT_EQ(ParseAndEvalExpression<double>(program), 9*2);
}

TEST(VM, For_loop_with_var_decl)
{
    std::string program =
            "double score = 1;"
            "for(double i=0; i<10; i=i+1 )"
            "{"
            "   score = score*2;"
            "}"
            "score;";
    EXPECT_EQ(ParseAndEvalExpression<double>(program), 1 * pow(2, 10) );
}

TEST(VM, declare_then_define ) {
    std::string program_01 =
            "double b;"
            "b = 5;"
            "b;";
    EXPECT_EQ(ParseAndEvalExpression<int>(program_01), 5);
}

TEST(VM, declare_and_define_then_reassign ) {
    std::string program_01 =
            "double b = 6;"
            "b = 5;"
            "b;";
    EXPECT_EQ(ParseAndEvalExpression<int>(program_01), 5);
}

TEST(VM, declare_then_define_then_reassign ) {
    std::string program_01 =
            "double b;"
            "b = 6;"
            "b = 5;"
            "b;";
    EXPECT_EQ(ParseAndEvalExpression<int>(program_01), 5);
}

TEST(VM, condition_which_contains_alterated_var ) {
    std::string program =
            "double b = 6;"
            "b = 5;"
            "string res = \"ok\";"
            "if( b==6 )"
            "{"
            "  res=\"error\";"
            "}"
            "res;";
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

