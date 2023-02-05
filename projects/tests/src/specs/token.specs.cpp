#include <gtest/gtest.h>
#include <ndbl/core/Property.h>

using namespace ndbl;

TEST(Token, empty_constructor)
{
    Token token;
    EXPECT_EQ(token.get_prefix(), "");
    EXPECT_EQ(token.get_word(), "");
    EXPECT_EQ(token.get_suffix(), "");
 }

TEST(Token, constructor)
{
    Token token(Token_t::identifier, "toto",0);
    EXPECT_EQ(token.get_prefix(), "");
    EXPECT_EQ(token.get_word(), "toto");
    EXPECT_EQ(token.get_suffix(), "");
}

TEST(Token, append_prefix)
{
    Token token(Token_t::identifier, "toto",0);
    token.append_to_prefix(" ");

    EXPECT_EQ(token.get_prefix(), " ");
    EXPECT_EQ(token.get_word(), "toto");
    EXPECT_EQ(token.get_suffix(), "");
}

TEST(Token, append_suffix)
{
    Token token(Token_t::identifier, "toto",0);
    token.append_to_suffix(" ");

    EXPECT_EQ(token.get_prefix(), "");
    EXPECT_EQ(token.get_word(), "toto");
    EXPECT_EQ(token.get_suffix(), " ");
}

TEST(Token, append_as_word)
{
    Token token(Token_t::identifier, "",0);
    token.append_to_prefix("pref ");
    token.append_to_word("fresh");
    EXPECT_EQ(token.m_buffer, "pref fresh");
    EXPECT_EQ(token.get_word(), "fresh");
}

TEST(Token, transfer_prefix_suffix)
{
    auto src_token = std::make_shared<Token>(Token_t::identifier, "TATA",0);
    src_token->append_to_prefix("pref ");
    src_token->append_to_suffix(" suff");

    Token token(Token_t::identifier, "TOTO",0);

    token.transfer_prefix_suffix( src_token );

    EXPECT_EQ( token.m_buffer, "pref TOTO suff");
    EXPECT_EQ( src_token->m_buffer, "TATA");
}

