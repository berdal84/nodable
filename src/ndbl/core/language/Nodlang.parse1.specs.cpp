#include "../fixtures/core.h"
#include <gtest/gtest.h>
#include "tools/core/Log.h"

using namespace ndbl;

typedef ::testing::Core Language_parse_token;

///////////////////////// Atomic expressions ///////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_token, atomic_expression_if)
{
    bdc::String buffer{"if"};
    Token token = lang_parse_token(language(), buffer);
    EXPECT_EQ(token.type, Token_Type_keyword_if);
    EXPECT_EQ(token.view(), "if");
}

TEST_F(Language_parse_token, atomic_expression_else)
{
    bdc::String buffer{"else"};
    Token token = lang_parse_token(language(), buffer);
    EXPECT_EQ(token.type, Token_Type_keyword_else);
    EXPECT_EQ(token.view(), "else");
}

TEST_F(Language_parse_token, atomic_expression_for)
{
    bdc::String buffer{"for"};
    Token token = lang_parse_token(language(), buffer);
    EXPECT_EQ(token.type, Token_Type_keyword_for);
    EXPECT_EQ(token.view(), "for");
}

TEST_F(Language_parse_token, atomic_expression_bool_true)
{
    bdc::String buffer{"true"};
    Token token = lang_parse_token(language(), buffer);
    EXPECT_EQ(token.type, Token_Type_literal_bool);
    EXPECT_EQ(token.view(), "true");
}

TEST_F(Language_parse_token, atomic_expression_bool_false)
{
    bdc::String buffer{"false"};
    Token token = lang_parse_token(language(), buffer);
    EXPECT_EQ(token.type, Token_Type_literal_bool);
    EXPECT_EQ(token.view(), "false");
}

TEST_F(Language_parse_token, atomic_expression_int_5)
{
    bdc::String buffer{"5"};
    Token token = lang_parse_token(language(), buffer);
    EXPECT_EQ(token.type, Token_Type_literal_int);
}

TEST_F(Language_parse_token, atomic_expression_double_5_0)
{
    bdc::String buffer{"5.0"};
    Token token = lang_parse_token(language(), buffer);
    EXPECT_EQ(token.type, Token_Type_literal_double);
    EXPECT_EQ(token.view(), "5.0");
}

TEST_F(Language_parse_token, atomic_expression_double_5_0001)
{
    bdc::String buffer{"5.0001"};
    Token token = lang_parse_token(language(), buffer);
    EXPECT_EQ(token.type, Token_Type_literal_double);
    EXPECT_EQ(token.view(), "5.0001");
}

TEST_F(Language_parse_token, atomic_expression_string)
{
    bdc::String buffer{"\"Hello\""};
    Token token = lang_parse_token(language(), buffer);
    EXPECT_EQ(token.type, Token_Type_literal_string);
    EXPECT_EQ(token.view(), "\"Hello\"");
}