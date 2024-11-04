#include "../fixtures/core.h"
#include <gtest/gtest.h>
#include "tools/core/log.h"
#include "ndbl/core/FunctionNode.h"

using namespace ndbl;

typedef ::testing::Core Language_parse_function_call;
using tools::Optional;

TEST_F(Language_parse_function_call, dna_to_protein)
{
    auto* language = app.get_language();

    // tokenize
    language->tokenize("dna_to_protein(\"GATACA\")");

    // check
    TokenRibbon& ribbon = language->_state.tokens();
    EXPECT_EQ(ribbon.size(), 4);
    EXPECT_EQ(ribbon.at(0).m_type, Token_t::identifier);
    EXPECT_EQ(ribbon.at(1).m_type, Token_t::parenthesis_open);
    EXPECT_EQ(ribbon.at(2).m_type, Token_t::literal_string);
    EXPECT_EQ(ribbon.at(3).m_type, Token_t::parenthesis_close);

    // parse
    Optional<Slot*> function_out = language->parse_function_call();

    // check
    EXPECT_TRUE(function_out.valid());
    EXPECT_TRUE(function_out->node->type() == NodeType_FUNCTION);
}

TEST_F(Language_parse_function_call, operator_add)
{
    auto* language = app.get_language();

    // tokenize
    language->tokenize("42+42");

    // check
    TokenRibbon& ribbon = language->_state.tokens();
    EXPECT_EQ(ribbon.size(), 3);
    EXPECT_EQ(ribbon.at(0).m_type, Token_t::literal_int);
    EXPECT_EQ(ribbon.at(1).m_type, Token_t::operator_);
    EXPECT_EQ(ribbon.at(2).m_type, Token_t::literal_int);

    // parse
    Optional<Slot*> result = language->parse_expression();

    // check
    EXPECT_TRUE(result.valid());
    EXPECT_TRUE(result->node->type() == NodeType_OPERATOR);
}