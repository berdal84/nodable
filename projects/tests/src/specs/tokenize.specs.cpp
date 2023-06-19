#include "../fixtures/core.h"
#include <gtest/gtest.h>
#include "fw/core/log.h"
#include <iostream>

using namespace ndbl;

typedef ::testing::Core tokenize;

//////////////////////////// Identifiers ///////////////////////////////////////////////////////////////////////////////

TEST_F(tokenize, identifiers_can_start_by_a_keyword)
{
    std::string code{"int if_myvar_includes_a_keyword;"};
    language.tokenize(code);
    log_ribbon();
    std::shared_ptr<Token> token = language.m_token_ribbon.tokens[1];
    EXPECT_EQ(token->word_to_string(), "if_myvar_includes_a_keyword");
    EXPECT_EQ(token->m_type, Token_t::identifier);
}

//////////////////////////// Prefix / Suffix ///////////////////////////////////////////////////////////////////////////

TEST_F(tokenize, identifiers_should_not_have_prefix_or_suffix)
{
    std::string code{"int my_var ;"};
    language.tokenize(code);
    log_ribbon();
    std::shared_ptr<Token> token = language.m_token_ribbon.tokens[1];
    EXPECT_EQ(token->word_to_string(), "my_var");
    EXPECT_EQ(token->prefix_to_string(), "");
    EXPECT_EQ(token->suffix_to_string(), "");
}

TEST_F(tokenize, operator_suffix_and_prefix)
{
    std::string code{"int my_var = 42"};
    language.tokenize(code);
    log_ribbon();
    std::shared_ptr<Token> token = language.m_token_ribbon.tokens[2];
    EXPECT_EQ(token->buffer_to_string(), " = ");
    EXPECT_EQ(token->prefix_to_string(), " ");
    EXPECT_EQ(token->suffix_to_string(), " ");
}

TEST_F(tokenize, operator_suffix)
{
    std::string code = "int my_var= 42";
    language.tokenize(code);
    log_ribbon();
    std::shared_ptr<Token> token = language.m_token_ribbon.tokens[2];
    EXPECT_EQ(token->buffer_to_string(), "= ");
    EXPECT_EQ(token->prefix_to_string(), "");
    EXPECT_EQ(token->suffix_to_string(), " ");
}

TEST_F(tokenize, operator_prefix)
{
    std::string code = "int my_var =42";
    language.tokenize(code);
    log_ribbon();
    std::shared_ptr<Token> token = language.m_token_ribbon.tokens[2];
    EXPECT_EQ(token->buffer_to_string(), " =");
    EXPECT_EQ(token->prefix_to_string(), " ");
    EXPECT_EQ(token->suffix_to_string(), "");
}