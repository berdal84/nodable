#include <gtest/gtest.h>
#include "ndbl/core/Property.h"

using namespace ndbl;

TEST(Token, empty_constructor)
{
    Token token;
    EXPECT_EQ(token.prefix_to_string(), "");
    EXPECT_EQ(token.word_to_string(), "");
    EXPECT_EQ(token.suffix_to_string(), "");
    EXPECT_EQ(token.buffer_to_string(), "");
 }

TEST(Token, constructor__with_const_char_ptr)
{
    Token token(Token_t::identifier, "toto");
    EXPECT_EQ(token.prefix_to_string(), "");
    EXPECT_EQ(token.word_to_string(), "toto");
    EXPECT_EQ(token.suffix_to_string(), "");
    EXPECT_EQ(token.buffer_to_string(), "toto");
    EXPECT_EQ(token.m_is_buffer_owned, false);
}

TEST(Token, suffix_append_from_stack)
{
    //                          >|--|<
    const char* toto = "// test\ntoto";
    Token token(Token_t::identifier, const_cast<char*>(toto), 8, 4 );

    EXPECT_EQ(token.prefix_to_string(), "");
    EXPECT_EQ(token.m_is_buffer_owned, false);

    token.suffix_append(";\n");

    EXPECT_EQ(token.suffix_to_string(), ";\n");
    EXPECT_EQ(token.buffer_to_string(), "toto;\n");
    EXPECT_EQ(token.m_word_start_pos, 0);
    EXPECT_EQ(token.m_is_buffer_owned, true);
}

TEST(Token, constructor__with_not_owned_buffer)
{
    const char* buffer = "<prefix>toto<suffix>";
    Token token(Token_t::identifier, const_cast<char*>(buffer), 0, strlen(buffer), 8, 4 );
    EXPECT_EQ(token.prefix_to_string(), "<prefix>");
    EXPECT_EQ(token.word_to_string(), "toto");
    EXPECT_EQ(token.suffix_to_string(), "<suffix>");
    EXPECT_EQ(token.m_is_buffer_owned, false);
}

TEST(Token, take_prefix_suffix_from)
{
    // prepare
    std::string toto{"TOTO"};
    std::string tata{"<prefix>TATA<suffix>"};
    Token source(Token_t::identifier, const_cast<char*>(tata.data()), 0, tata.length(), 8, 4);
    Token target(Token_t::identifier, const_cast<char*>(toto.data()), 0, toto.length());

    // pre-check
    EXPECT_EQ(source.buffer_to_string(), "<prefix>TATA<suffix>");
    EXPECT_FALSE(source.m_is_buffer_owned);

    EXPECT_EQ(target.buffer_to_string(), "TOTO");
    EXPECT_FALSE(target.m_is_buffer_owned);

    // act
    target.take_prefix_suffix_from(&source);

    // post-check
    EXPECT_EQ(source.buffer_to_string(), "TATA");
    EXPECT_FALSE(source.m_is_buffer_owned);

    EXPECT_EQ(target.buffer_to_string(), "<prefix>TOTO<suffix>");
    EXPECT_TRUE(target.m_is_buffer_owned);
}

TEST(Token, replace_word__same_length)
{
    // prepare
    std::string tata{"<prefix>TATA<suffix>"};
    Token source(Token_t::identifier, const_cast<char*>(tata.data()), 0, tata.length(), 8, 4);

    // pre-check
    EXPECT_EQ(source.buffer_to_string(), "<prefix>TATA<suffix>");
    EXPECT_FALSE(source.m_is_buffer_owned);

    // act
    source.word_replace("TOTO");

    // post-check
    EXPECT_EQ(source.buffer_to_string(), "<prefix>TOTO<suffix>");
    EXPECT_TRUE(source.m_is_buffer_owned);
}

TEST(Token, replace_word__larger)
{
    // prepare
    const char* tata = "<prefix>42<suffix>";
    Token source(Token_t::identifier, const_cast<char*>(tata), 0, strlen(tata), 8, 2);

    // pre-check
    EXPECT_EQ(source.buffer_to_string(), "<prefix>42<suffix>");
    EXPECT_EQ(source.word_to_string(), "42");
    EXPECT_FALSE(source.m_is_buffer_owned);

    // act
    source.word_replace("2048");

    // post-check
    EXPECT_EQ(source.buffer_to_string(), "<prefix>2048<suffix>");
    EXPECT_TRUE(source.m_is_buffer_owned);
}


TEST(Token, replace_word__smaller)
{
    // prepare
    const char* tata = "<prefix>42<suffix>";
    Token source(Token_t::identifier, const_cast<char*>(tata), 0, strlen(tata), 8, 2);

    // pre-check
    EXPECT_EQ(source.buffer_to_string(), "<prefix>42<suffix>");
    EXPECT_EQ(source.word_to_string(), "42");
    EXPECT_FALSE(source.m_is_buffer_owned);

    // act
    source.word_replace("0");

    // post-check
    EXPECT_EQ(source.buffer_to_string(), "<prefix>0<suffix>");
    EXPECT_TRUE(source.m_is_buffer_owned);
}