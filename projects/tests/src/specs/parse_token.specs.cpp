#include "../fixtures/core.h"
#include <gtest/gtest.h>
#include "fw/core/log.h"

using namespace ndbl;

typedef ::testing::Core parse_token;

///////////////////////// Atomic expressions ///////////////////////////////////////////////////////////////////////////

TEST_F(parse_token, atomic_expression_bool_true)
{
    std::string buffer{"true"};
    std::shared_ptr<Token> token = language.parse_token(buffer);
    EXPECT_EQ(token->m_type, Token_t::literal_bool);
    EXPECT_EQ(token->buffer_to_string(), "true");
}

TEST_F(parse_token, atomic_expression_bool_false)
{
    std::string buffer{"false"};
    std::shared_ptr<Token> token = language.parse_token(buffer);
    EXPECT_EQ(token->m_type, Token_t::literal_bool);
    EXPECT_EQ(token->buffer_to_string(), "false");
}

TEST_F(parse_token, atomic_expression_int_5)
{
    std::string buffer{"5"};
    std::shared_ptr<Token> token = language.parse_token(buffer);
    EXPECT_EQ(token->m_type, Token_t::literal_int);
}

TEST_F(parse_token, atomic_expression_double_5_0)
{
    std::string buffer{"5.0"};
    std::shared_ptr<Token> token = language.parse_token(buffer);
    EXPECT_EQ(token->m_type, Token_t::literal_double);
    EXPECT_EQ(token->buffer_to_string(), "5.0");
}

TEST_F(parse_token, atomic_expression_double_5_0001)
{
    std::string buffer{"5.0001"};
    std::shared_ptr<Token> token = language.parse_token(buffer);
    EXPECT_EQ(token->m_type, Token_t::literal_double);
    EXPECT_EQ(token->buffer_to_string(), "5.0001");
}

TEST_F(parse_token, atomic_expression_string)
{
    std::string buffer{"\"Hello\""};
    std::shared_ptr<Token> token = language.parse_token(buffer);
    EXPECT_EQ(token->m_type, Token_t::literal_string);
    EXPECT_EQ(token->buffer_to_string(), "\"Hello\"");
}