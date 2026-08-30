#include "../fixtures/core.h"
#include <gtest/gtest.h>
#include <iostream>

using namespace ndbl;

typedef ::testing::Core Language_tokenize;

//////////////////////////// Identifiers ///////////////////////////////////////////////////////////////////////////////

TEST_F(Language_tokenize, identifiers_can_start_by_a_keyword)
{
    bdc::String code = "int if_myvar_includes_a_keyword;";
    lang_tokenize(language(), code);
    log_ribbon();
    const Token& token = language().ribbon[1];
    EXPECT_EQ(token.word_view, "if_myvar_includes_a_keyword");
    EXPECT_EQ(token.type, Token_Type_identifier);
}

//////////////////////////// Prefix / Suffix ///////////////////////////////////////////////////////////////////////////

TEST_F(Language_tokenize, identifiers_should_not_have_prefix_or_suffix)
{
    bdc::String code{"int my_var ;"};
    lang_tokenize(language(), code);
    log_ribbon();
    const Token& token = language().ribbon[1];
    EXPECT_EQ(token.word_view, "my_var");
    EXPECT_EQ(token.prefix_view, "");
    EXPECT_EQ(token.suffix_view, "");
}

TEST_F(Language_tokenize, operator_suffix_and_prefix)
{
    bdc::String code{"int my_var = 42"};
    lang_tokenize(language(), code);
    log_ribbon();
    const Token& token = language().ribbon[2];
    EXPECT_EQ(token.buffer     , " = ");
    EXPECT_EQ(token.prefix_view, " ");
    EXPECT_EQ(token.suffix_view, " ");
}

TEST_F(Language_tokenize, operator_suffix)
{
    bdc::String code = "int my_var= 42";
    lang_tokenize(language(), code);
    log_ribbon();
    const Token& token = language().ribbon[2];
    EXPECT_EQ(token.buffer, "= ");
    EXPECT_EQ(token.prefix_view, "");
    EXPECT_EQ(token.suffix_view, " ");
}

TEST_F(Language_tokenize, operator_prefix)
{
    bdc::String code = "int my_var =42";
    lang_tokenize(language(), code);
    log_ribbon();
    const Token& token = language().ribbon[2];
    EXPECT_EQ(token.buffer   , " =");
    EXPECT_EQ(token.prefix_view, " " );
    EXPECT_EQ(token.suffix_view, ""  );
}


TEST_F(Language_tokenize, add_pow2of2_and_integer )
{
    bdc::String code = "pow(2,2) + 1";
    lang_tokenize(language(), code);
    log_ribbon();
    EXPECT_EQ(language().ribbon[2].buffer, "2");
    EXPECT_EQ(language().ribbon[3].buffer, ",");
    EXPECT_EQ(language().ribbon[4].buffer, "2");
    EXPECT_EQ(language().ribbon[5].buffer, ")"); // parser should not add a " " prefix after ")"
    EXPECT_EQ(language().ribbon[6].buffer, " + ");
    EXPECT_EQ(language().ribbon[7].buffer, "1");

}

TEST_F(Language_tokenize, return_integer )
{
    bdc::String code = "return 42";
    lang_tokenize(language(), code);
    EXPECT_EQ(language().ribbon[0].word_view, "return");
    EXPECT_EQ(language().ribbon[1].word_view, "42");

}