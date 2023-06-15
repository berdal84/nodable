#include "../fixtures/core.h"
#include <gtest/gtest.h>
#include "fw/core/log.h"

using namespace ndbl;

typedef ::testing::Core parse_token;

///////////////////////// Atomic expressions ///////////////////////////////////////////////////////////////////////////

TEST_F(parse_token, atomic_expression_bool_true)
{
    std::shared_ptr<Token> token = language.parse_token("true");
    EXPECT_EQ(token->m_type, Token_t::literal_bool);
    EXPECT_EQ(token->m_buffer, "true");
}

TEST_F(parse_token, atomic_expression_bool_false)
{
    std::shared_ptr<Token> token = language.parse_token("false");
    EXPECT_EQ(token->m_type, Token_t::literal_bool);
    EXPECT_EQ(token->m_buffer, "false");
}

TEST_F(parse_token, atomic_expression_int_5)
{
    std::shared_ptr<Token> token = language.parse_token("5");
    EXPECT_EQ(token->m_type, Token_t::literal_int);
}

TEST_F(parse_token, atomic_expression_double_5_0)
{
    std::shared_ptr<Token> token = language.parse_token("5.0");
    EXPECT_EQ(token->m_type, Token_t::literal_double);
    EXPECT_EQ(token->m_buffer, "5.0");
}

TEST_F(parse_token, atomic_expression_double_5_0001)
{
    std::shared_ptr<Token> token = language.parse_token("5.0001");
    EXPECT_EQ(token->m_type, Token_t::literal_double);
    EXPECT_EQ(token->m_buffer, "5.0001");
}

TEST_F(parse_token, atomic_expression_string)
{
    std::shared_ptr<Token> token = language.parse_token(R"("Hello")");
    EXPECT_EQ(token->m_type, Token_t::literal_string);
    EXPECT_EQ(token->m_buffer, R"("Hello")");
}