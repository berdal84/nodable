#include <gtest/gtest.h>
#include "ndbl/core/Node_Property.h"

using namespace ndbl;
using namespace bdc;

TEST(Token, empty_constructor)
{
    Token token;

    EXPECT_EQ(token.prefix_view()  , "");
    EXPECT_EQ(token.word_view()     , "");
    EXPECT_EQ(token.suffix_view()   , "");
    EXPECT_EQ(token.view()          , "");
 }

TEST(Token, constructor__with_const_char_ptr)
{
    Token token(Token_Type_identifier, "toto");

    EXPECT_EQ(token.prefix_view()   , "");
    EXPECT_EQ(token.word_view()     , "toto");
    EXPECT_EQ(token.suffix_view()   , "");
    EXPECT_EQ(token.view()          , "toto");
    EXPECT_EQ(token.owns_data       , false);
}

TEST(Token, suffix_append_from_stack)
{
    //                     >|--|<
    String toto = "// test\ntoto"; toto = string_advance(toto, 8);
    Token token(Token_Type_identifier, toto );
    EXPECT_EQ(token.prefix_view()   , "");
    EXPECT_EQ(token.owns_data       , false);

    token.suffix_push_back(";\n");

    EXPECT_EQ(token.suffix_view()   , ";\n");
    EXPECT_EQ(token.view()          , "toto;\n");
    EXPECT_EQ(token.owns_data       , true);
}

TEST(Token, constructor__with_not_owned_buffer)
{
    String buffer = "<prefix>toto<suffix>";
    Token token(Token_Type_identifier, buffer);
    token.word_move_begin(8);
    token.word_move_end(-8);

    EXPECT_EQ(token.prefix_view()   , "<prefix>");
    EXPECT_EQ(token.word_view()     , "toto");
    EXPECT_EQ(token.suffix_view()   , "<suffix>");
    EXPECT_EQ(token.owns_data       , false);
}

TEST(Token, take_prefix_suffix_from)
{
    // prepare

    bdc::String tata = "<prefix>TATA<suffix>";
    Token source(Token_Type_identifier, tata);
    source.word_move_begin(8);
    source.word_move_end(-8);

    bdc::String toto = "TOTO";
    Token target(Token_Type_identifier, toto);

    // pre-check
    EXPECT_EQ(source.prefix_view()  , "<prefix>");
    EXPECT_EQ(source.word_view()    , "TATA");
    EXPECT_EQ(source.suffix_view()  , "<suffix>");
    EXPECT_FALSE(source.owns_data);

    EXPECT_EQ(target.view()         , "TOTO");
    EXPECT_FALSE(target.owns_data);

    // act
    target.take_prefix_suffix_from(&source);

    // post-check
    EXPECT_EQ(source.prefix_view()  , "");
    EXPECT_EQ(source.word_view()    , "TATA");
    EXPECT_EQ(source.suffix_view()  , "");
    EXPECT_FALSE(source.owns_data);

    EXPECT_EQ(target.view()         , "<prefix>TOTO<suffix>");
    EXPECT_TRUE(target.owns_data);
}

TEST(Token, replace_word__same_length)
{
    // prepare
    bdc::String tata = "<prefix>TATA<suffix>";
    Token source(Token_Type_identifier, tata);
    source.word_move_begin(8);
    source.word_move_end(-8);

    // pre-check
    EXPECT_EQ(source.view(), "<prefix>TATA<suffix>");
    EXPECT_FALSE(source.owns_data);

    // act
    source.replace_word("TOTO");

    // post-check
    EXPECT_EQ(source.view(), "<prefix>TOTO<suffix>");
    EXPECT_TRUE(source.owns_data);
}

TEST(Token, replace_word__larger)
{
    // prepare
    String tata = "<prefix>42<suffix>";
    Token source(Token_Type_identifier, tata);
    source.word_move_begin(8);
    source.word_move_end(-8);

    // pre-check
    EXPECT_EQ(source.view()     , "<prefix>42<suffix>");
    EXPECT_EQ(source.word_view(), "42");
    EXPECT_FALSE(source.owns_data);

    // act
    source.replace_word("2048");

    // post-check
    EXPECT_EQ(source.view(), "<prefix>2048<suffix>");
    EXPECT_TRUE(source.owns_data);
}


TEST(Token, replace_word__smaller)
{
    // prepare
    String tata = "<prefix>42<suffix>";
    Token source(Token_Type_identifier, tata);
    source.word_move_begin(8);
    source.word_move_end(-8);

    // pre-check
    EXPECT_EQ(source.view()     , "<prefix>42<suffix>");
    EXPECT_EQ(source.word_view(), "42");
    EXPECT_FALSE(source.owns_data);

    // act
    source.replace_word("0");

    // post-check
    EXPECT_EQ(source.view(), "<prefix>0<suffix>");
    EXPECT_TRUE(source.owns_data);
}