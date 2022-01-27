#include <gtest/gtest.h>
#include <nodable/Log.h>
#include "../tools.h"

using namespace Nodable;

TEST(Runner, Cond_1 )
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

TEST(Runner, Cond_2 )
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

TEST(Runner, Loop_1 )
{
    std::string program =
            "double n   = 0;"
            "string res = \"\";"
            "for(n=0;n<10;n=n+1){"
            "   res = res + n;"
            "}"
            "res;";

    EXPECT_EQ( ParseAndEvalExpression<std::string>(program), "0123456789" );
}