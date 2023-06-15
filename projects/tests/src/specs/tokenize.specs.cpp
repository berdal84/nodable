#include "../fixtures/core.h"
#include <gtest/gtest.h>
#include "fw/core/log.h"

using namespace ndbl;

typedef ::testing::Core tokenize;

//////////////////////////// Identifiers ///////////////////////////////////////////////////////////////////////////////

TEST_F(tokenize, identifiers_can_start_by_a_keyword)
{
    language.tokenize("int if_myvar_includes_a_keyword;");
    std::shared_ptr<Token> token = language.m_token_ribbon.tokens[1];
    EXPECT_EQ(token->get_word(), "if_myvar_includes_a_keyword");
    EXPECT_EQ(token->m_type, Token_t::identifier);
}

//////////////////////////// Prefix / Suffix ///////////////////////////////////////////////////////////////////////////

TEST_F(tokenize, identifiers_should_not_have_prefix_or_suffix)
{
    language.tokenize("int my_var = 42");
    std::shared_ptr<Token> token = language.m_token_ribbon.tokens[1];
    EXPECT_EQ(token->m_buffer, "my_var");
    EXPECT_EQ(token->get_prefix(), "");
    EXPECT_EQ(token->get_suffix(), "");
}

TEST_F(tokenize, operator_suffix_and_prefix)
{
    language.tokenize("int my_var = 42");
    std::shared_ptr<Token> token = language.m_token_ribbon.tokens[2];
    EXPECT_EQ(token->m_buffer, " = ");
    EXPECT_EQ(token->get_prefix(), " ");
    EXPECT_EQ(token->get_suffix(), " ");
}

TEST_F(tokenize, operator_suffix)
{
    language.tokenize("int my_var= 42");
    std::shared_ptr<Token> token = language.m_token_ribbon.tokens[2];
    EXPECT_EQ(token->m_buffer, "= ");
    EXPECT_EQ(token->get_prefix(), "");
    EXPECT_EQ(token->get_suffix(), " ");
}

TEST_F(tokenize, operator_prefix)
{
    language.tokenize("int my_var =42");
    std::shared_ptr<Token> token = language.m_token_ribbon.tokens[2];
    EXPECT_EQ(token->m_buffer, " =");
    EXPECT_EQ(token->get_prefix(), " ");
    EXPECT_EQ(token->get_suffix(), "");
}