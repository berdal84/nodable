#include "../fixtures/core.h"
#include <gtest/gtest.h>
#include <iostream>

using namespace ndbl;

typedef ::testing::Core Language_tokenize;

//////////////////////////// Identifiers ///////////////////////////////////////////////////////////////////////////////

TEST_F(Language_tokenize, identifiers_can_start_by_a_keyword)
{
    std::string code{"int if_myvar_includes_a_keyword;"};
    language->tokenize(code);
    log_ribbon();
    Token token = language->parser_state.ribbon.at(1);
    EXPECT_EQ(token.word_to_string(), "if_myvar_includes_a_keyword");
    EXPECT_EQ(token.m_type, Token_t::identifier);
}

//////////////////////////// Prefix / Suffix ///////////////////////////////////////////////////////////////////////////

TEST_F(Language_tokenize, identifiers_should_not_have_prefix_or_suffix)
{
    std::string code{"int my_var ;"};
    language->tokenize(code);
    log_ribbon();
    Token token = language->parser_state.ribbon.at(1);
    EXPECT_EQ(token.word_to_string(), "my_var");
    EXPECT_EQ(token.prefix_to_string(), "");
    EXPECT_EQ(token.suffix_to_string(), "");
}

TEST_F(Language_tokenize, operator_suffix_and_prefix)
{
    std::string code{"int my_var = 42"};
    language->tokenize(code);
    log_ribbon();
    Token token = language->parser_state.ribbon.at(2);
    EXPECT_EQ(token.buffer_to_string(), " = ");
    EXPECT_EQ(token.prefix_to_string(), " ");
    EXPECT_EQ(token.suffix_to_string(), " ");
}

TEST_F(Language_tokenize, operator_suffix)
{
    std::string code = "int my_var= 42";
    language->tokenize(code);
    log_ribbon();
    Token token = language->parser_state.ribbon.at(2);
    EXPECT_EQ(token.buffer_to_string(), "= ");
    EXPECT_EQ(token.prefix_to_string(), "");
    EXPECT_EQ(token.suffix_to_string(), " ");
}

TEST_F(Language_tokenize, operator_prefix)
{
    std::string code = "int my_var =42";
    language->tokenize(code);
    log_ribbon();
    Token token = language->parser_state.ribbon.at(2);
    EXPECT_EQ(token.buffer_to_string(), " =");
    EXPECT_EQ(token.prefix_to_string(), " ");
    EXPECT_EQ(token.suffix_to_string(), "");
}


TEST_F(Language_tokenize, add_pow2of2_and_integer )
{
    std::string code = "pow(2,2) + 1";
    language->tokenize(code);
    TokenRibbon& ribbon = language->parser_state.ribbon;
    EXPECT_EQ(ribbon.at(2).buffer_to_string(), "2");
    EXPECT_EQ(ribbon.at(3).buffer_to_string(), ",");
    EXPECT_EQ(ribbon.at(4).buffer_to_string(), "2");
    EXPECT_EQ(ribbon.at(5).buffer_to_string(), ")"); // parser should not add a " " prefix after ")"
    EXPECT_EQ(ribbon.at(6).buffer_to_string(), " + ");
    EXPECT_EQ(ribbon.at(7).buffer_to_string(), "1");

}