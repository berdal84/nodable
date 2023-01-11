#include <gtest/gtest.h>

#include <nodable/core/Member.h>
#include <nodable/core/Node.h>
#include <nodable/core/GraphNode.h>
#include <nodable/core/InstructionNode.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/NodeFactory.h>
#include <nodable/core/Wire.h>
#include <nodable/core/languages/NodableLanguage.h>
#include <nodable/core/InvokableComponent.h>
#include <nodable/core/Scope.h>
#include <nodable/core/reflection/func_type.h>

using namespace ndbl;

TEST( token, empty_constructor)
{
    Token token;
    EXPECT_EQ(token.get_prefix(), "");
    EXPECT_EQ(token.get_word(), "");
    EXPECT_EQ(token.get_suffix(), "");
 }

TEST( token, constructor)
{
    Token token(Token_t::identifier, "toto",0);
    EXPECT_EQ(token.get_prefix(), "");
    EXPECT_EQ(token.get_word(), "toto");
    EXPECT_EQ(token.get_suffix(), "");
}

TEST( token, append_prefix)
{
    Token token(Token_t::identifier, "toto",0);
    token.append_to_prefix(" ");

    EXPECT_EQ(token.get_prefix(), " ");
    EXPECT_EQ(token.get_word(), "toto");
    EXPECT_EQ(token.get_suffix(), "");
}

TEST( token, append_suffix)
{
    Token token(Token_t::identifier, "toto",0);
    token.append_to_suffix(" ");

    EXPECT_EQ(token.get_prefix(), "");
    EXPECT_EQ(token.get_word(), "toto");
    EXPECT_EQ(token.get_suffix(), " ");
}

TEST( token, append_as_word)
{
    Token token(Token_t::identifier, "",0);
    token.append_to_prefix("pref ");
    token.append_to_word("fresh");
    EXPECT_EQ(token.m_buffer, "pref fresh");
    EXPECT_EQ(token.get_word(), "fresh");
}

TEST( token, transfer_prefix_suffix)
{
    auto src_token = std::make_shared<Token>(Token_t::identifier, "TATA",0);
    src_token->append_to_prefix("pref ");
    src_token->append_to_suffix(" suff");

    Token token(Token_t::identifier, "TOTO",0);

    token.transfer_prefix_suffix( src_token );

    EXPECT_EQ( token.m_buffer, "pref TOTO suff");
    EXPECT_EQ( src_token->m_buffer, "TATA");
}

