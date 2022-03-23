#include <gtest/gtest.h>
#include <nodable/core/Log.h>
#include "../tools.h"

using namespace Nodable;

TEST(VM, Cond_1 )
{
    std::string program =
            "double bob   = 50;"
            "double alice = 10;"
            "string val;"
            "if(bob>alice){"
            "   val = \"true\";"
            "}else{"
            "   val = \"false\";"
            "}"
            "val;"; // return-like

    EXPECT_EQ( ParseAndEvalExpression<std::string>(program), "true" );
}

TEST(VM, Cond_2 )
{
    std::string program =
            "double bob   = 0;"
            "double alice = 10;"
            "string val = \"default\";"
            "if(bob>alice){"
            "   val = \"true\";"
            "}else if (false) {"
            "   val = \"false\";"
            "}"
            "val;"; // return-like

    EXPECT_EQ( ParseAndEvalExpression<std::string>(program), "default" );
}

TEST(VM, Loop_1_using_global_var )
{
    std::string program =
            "string res = \"\";" \
            "for(double n=0;n<10;n=n+1){"
            "   res = res + to_string(n);"
            "}"
            "res;";

    EXPECT_EQ( ParseAndEvalExpression<std::string>(program), "0123456789" );
}

TEST(VM, Loop_1_using_local_var )
{
    std::string program =
            "string res = \"\";" \
            "for(double n=0;n<10;n=n+1){"
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
            "for(n=0;n<10;n=n+1){"
            "   p = pow(n,2);"
            "   if( p != n ){         /* skip powers equals to n */"
            "      res = res + p;     /* concat powers */"
            "   }else{"
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
            "for(double n=0;n<10;n=n+1){"
            "   double p = pow(n,2);"
            "   if( p != n ){         /* skip powers equals to n */"
            "      res = res + p;     /* concat powers */"
            "   }else{"
            "      res = res + \"_\"; /* concat \"_\" */"
            "   }"
            "}"
            "res;";

    EXPECT_EQ( ParseAndEvalExpression<std::string>(program), "__49162536496481" );
}
