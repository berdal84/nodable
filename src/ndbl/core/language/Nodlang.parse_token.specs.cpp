#include "../fixtures/core.h"
#include <gtest/gtest.h>
#include "tools/core/log.h"

using namespace ndbl;

typedef ::testing::Core Language_parse_token;

///////////////////////// Atomic expressions ///////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_token, atomic_expression_if)
{
    std::string buffer{"if"};
    ASTToken token = language->parse_token(buffer);
    EXPECT_EQ(token.m_type, TokenType::keyword_if);
    EXPECT_EQ(token.buffer_to_string(), "if");
}

TEST_F(Language_parse_token, atomic_expression_else)
{
    std::string buffer{"else"};
    ASTToken token = language->parse_token(buffer);
    EXPECT_EQ(token.m_type, TokenType::keyword_else);
    EXPECT_EQ(token.buffer_to_string(), "else");
}

TEST_F(Language_parse_token, atomic_expression_for)
{
    std::string buffer{"for"};
    ASTToken token = language->parse_token(buffer);
    EXPECT_EQ(token.m_type, TokenType::keyword_for);
    EXPECT_EQ(token.buffer_to_string(), "for");
}

TEST_F(Language_parse_token, atomic_expression_bool_true)
{
    std::string buffer{"true"};
    ASTToken token = language->parse_token(buffer);
    EXPECT_EQ(token.m_type, TokenType::literal_bool);
    EXPECT_EQ(token.buffer_to_string(), "true");
}

TEST_F(Language_parse_token, atomic_expression_bool_false)
{
    std::string buffer{"false"};
    ASTToken token = language->parse_token(buffer);
    EXPECT_EQ(token.m_type, TokenType::literal_bool);
    EXPECT_EQ(token.buffer_to_string(), "false");
}

TEST_F(Language_parse_token, atomic_expression_int_5)
{
    std::string buffer{"5"};
    ASTToken token = language->parse_token(buffer);
    EXPECT_EQ(token.m_type, TokenType::literal_int);
}

TEST_F(Language_parse_token, atomic_expression_double_5_0)
{
    std::string buffer{"5.0"};
    ASTToken token = language->parse_token(buffer);
    EXPECT_EQ(token.m_type, TokenType::literal_double);
    EXPECT_EQ(token.buffer_to_string(), "5.0");
}

TEST_F(Language_parse_token, atomic_expression_double_5_0001)
{
    std::string buffer{"5.0001"};
    ASTToken token = language->parse_token(buffer);
    EXPECT_EQ(token.m_type, TokenType::literal_double);
    EXPECT_EQ(token.buffer_to_string(), "5.0001");
}

TEST_F(Language_parse_token, atomic_expression_string)
{
    std::string buffer{"\"Hello\""};
    ASTToken token = language->parse_token(buffer);
    EXPECT_EQ(token.m_type, TokenType::literal_string);
    EXPECT_EQ(token.buffer_to_string(), "\"Hello\"");
}