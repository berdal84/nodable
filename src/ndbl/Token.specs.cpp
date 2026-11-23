#include <gtest/gtest.h>
#include "ndbl/Node_Property.h"
#include "test/fixtures/basic_test.h"

typedef ::testing::Basic_Test Token_;

using namespace ndbl;
using namespace bdc;

TEST_F(Token_, empty_constructor)
{
    Token token;

    EXPECT_EQ(token_prefix_view(token)   , string(""));
    EXPECT_EQ(token_word_view(token)     , string(""));
    EXPECT_EQ(token_suffix_view(token)   , string(""));
    EXPECT_EQ(token_view(token)          , string(""));
 }

TEST_F(Token_, constructor__with_const_char_ptr)
{
    Token token(Token_Type_identifier, string("toto"));

    EXPECT_EQ(token_prefix_view(token)   , string(""));
    EXPECT_EQ(token_word_view(token)     , string("toto"));
    EXPECT_EQ(token_suffix_view(token)   , string(""));
    EXPECT_EQ(token_view(token)          , string("toto"));
    EXPECT_EQ(token.owns_data            , false);
}

TEST_F(Token_, suffix_append_from_stack)
{
    //                            >|--|<
    String toto = string("// test\ntoto");
    toto = string_advance(toto, 8);

    Token token(Token_Type_identifier, toto );
    EXPECT_EQ(token_prefix_view(token), string(""));
    EXPECT_EQ(token.owns_data         , false);

    token_suffix_push_back(token, string(";\n"));

    EXPECT_EQ(token_suffix_view(token), string(";\n"));
    EXPECT_EQ(token_view(token)       , string("toto;\n"));
    EXPECT_EQ(token.owns_data         , true);
}

TEST_F(Token_, constructor__with_not_owned_buffer)
{
    String buffer = string("<prefix>toto<suffix>");
    Token token(Token_Type_identifier, buffer);
    token_ltrim_word(token, strlen("<prefix>"));
    token_rtrim_word(token, strlen("<suffix>"));

    EXPECT_EQ(token_prefix_view(token)   , string("<prefix>"));
    EXPECT_EQ(token_word_view(token)     , string("toto"));
    EXPECT_EQ(token_suffix_view(token)   , string("<suffix>"));
    EXPECT_EQ(token.owns_data            , false);
}

TEST_F(Token_, take_prefix_suffix_from)
{
    // prepare

    bdc::String tata = string("<prefix>TATA<suffix>");
    Token token(Token_Type_identifier, tata);
    token_ltrim_word(token, strlen("<prefix>"));
    token_rtrim_word(token, strlen("<suffix>"));

    bdc::String toto = string("TOTO");
    Token target(Token_Type_identifier, toto);

    // pre-check
    EXPECT_EQ(token_prefix_view(token)  , string("<prefix>"));
    EXPECT_EQ(token_word_view(token)    , string("TATA"));
    EXPECT_EQ(token_suffix_view(token)  , string("<suffix>"));
    EXPECT_FALSE(token.owns_data);

    EXPECT_EQ(token_view(target)         , string("TOTO"));
    EXPECT_FALSE(target.owns_data);

    // act
    token_take_prefix_suffix_from(target, token);

    // post-check
    EXPECT_EQ(token_prefix_view(token)  , string(""));
    EXPECT_EQ(token_word_view(token)    , string("TATA"));
    EXPECT_EQ(token_suffix_view(token)  , string(""));
    EXPECT_FALSE(token.owns_data);

    EXPECT_EQ(token_view(target)         , string("<prefix>TOTO<suffix>"));
    EXPECT_TRUE(target.owns_data);
}

TEST_F(Token_, replace_word__same_length)
{
    // prepare
    bdc::String tata = string("<prefix>TATA<suffix>");
    Token token(Token_Type_identifier, tata);
    token_ltrim_word(token, strlen("<prefix>"));
    token_rtrim_word(token, strlen("<suffix>"));

    // pre-check
    EXPECT_EQ(token_view(token), string("<prefix>TATA<suffix>"));
    EXPECT_FALSE(token.owns_data);

    // act
    token_replace_word(token, string("TOTO"));

    // post-check
    EXPECT_EQ(token_view(token), string("<prefix>TOTO<suffix>"));
    EXPECT_TRUE(token.owns_data);
}

TEST_F(Token_, replace_word__larger)
{
    // prepare
    String tata = string("<prefix>42<suffix>");
    Token token(Token_Type_identifier, tata);
    token_ltrim_word(token, strlen("<prefix>"));
    token_rtrim_word(token, strlen("<suffix>"));

    // pre-check
    EXPECT_EQ(token_view(token)     , string("<prefix>42<suffix>"));
    EXPECT_EQ(token_word_view(token), string("42"));
    EXPECT_FALSE(token.owns_data);

    // act
    token_replace_word(token, string("2048"));

    // post-check
    EXPECT_EQ(token_view(token), string("<prefix>2048<suffix>"));
    EXPECT_TRUE(token.owns_data);
}


TEST_F(Token_, replace_word__smaller)
{
    // prepare
    String tata = string("<prefix>42<suffix>");
    Token token(Token_Type_identifier, tata);
    token_ltrim_word(token, strlen("<prefix>"));
    token_rtrim_word(token, strlen("<suffix>"));

    // pre-check
    EXPECT_EQ(token_view(token)     , string("<prefix>42<suffix>"));
    EXPECT_EQ(token_word_view(token), string("42"));
    EXPECT_FALSE(token.owns_data);

    // act
    token_replace_word(token, string("0"));

    // post-check
    EXPECT_EQ(token_view(token), string("<prefix>0<suffix>"));
    EXPECT_TRUE(token.owns_data);
}