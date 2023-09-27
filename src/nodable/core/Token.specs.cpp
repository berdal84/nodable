#include <gtest/gtest.h>
#include "nodable/core/Property.h"

using namespace ndbl;

TEST(Token, empty_constructor)
{
    Token token;
    EXPECT_EQ(token.prefix_to_string(), "");
    EXPECT_EQ(token.word_to_string(), "");
    EXPECT_EQ(token.suffix_to_string(), "");
    EXPECT_EQ(token.buffer_to_string(), "");
 }

TEST(Token, constructor__with_owned_buffer)
{
    Token token(Token_t::identifier, "toto");
    EXPECT_EQ(token.prefix_to_string(), "");
    EXPECT_EQ(token.word_to_string(), "toto");
    EXPECT_EQ(token.suffix_to_string(), "");
    EXPECT_EQ(token.m_is_source_buffer_owned, true);
}

TEST(Token, constructor__with_not_owned_buffer)
{
    const char* buffer = "<prefix>toto<suffix>";
    Token token(Token_t::identifier, const_cast<char*>(buffer), 0, strlen(buffer), 8, 4 );
    EXPECT_EQ(token.prefix_to_string(), "<prefix>");
    EXPECT_EQ(token.word_to_string(), "toto");
    EXPECT_EQ(token.suffix_to_string(), "<suffix>");
    EXPECT_EQ(token.m_is_source_buffer_owned, false);
}

TEST(Token, transfer_prefix_suffix)
{
    // prepare
    std::string toto{"TOTO"};
    std::string tata{"<prefix>TATA<suffix>"};
    Token source(Token_t::identifier, const_cast<char*>(tata.data()), 0, tata.length(), 8, 4);
    Token target(Token_t::identifier, const_cast<char*>(toto.data()), 0, toto.length());

    // pre-check
    EXPECT_EQ(source.buffer_to_string(), "<prefix>TATA<suffix>");
    EXPECT_FALSE(source.m_is_source_buffer_owned);

    EXPECT_EQ(target.buffer_to_string(), "TOTO");
    EXPECT_FALSE(target.m_is_source_buffer_owned);

    // act
    target.move_prefixsuffix( &source );

    // post-check
    EXPECT_EQ(source.buffer_to_string(), "TATA");
    EXPECT_FALSE(source.m_is_source_buffer_owned);

    EXPECT_EQ(target.buffer_to_string(), "<prefix>TOTO<suffix>");
    EXPECT_TRUE(target.m_is_source_buffer_owned);
}
