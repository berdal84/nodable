#include "../fixtures/core.h"
#include <gtest/gtest.h>
#include "tools/core/log.h"
#include "ndbl/core/FunctionNode.h"

using namespace ndbl;

typedef ::testing::Core Language_parse_function_call;
using tools::Optional;

TEST_F(Language_parse_function_call, dna_to_protein)
{
    // prepare parser
    auto* language = app.get_language();
    std::string buffer{"dna_to_protein(\"GATACA\")"};
    Nodlang::ParserState& parser_state = language->parser_state;
    parser_state.clear();
    parser_state.set_source_buffer(buffer.data(), buffer.length());
    parser_state.graph = app.get_graph();

    // tokenize
    app.get_language()->tokenize(buffer);

    // check
    EXPECT_EQ( parser_state.ribbon.size(), 4);
    EXPECT_EQ( parser_state.ribbon.at(0).m_type, Token_t::identifier);
    EXPECT_EQ( parser_state.ribbon.at(1).m_type, Token_t::parenthesis_open);
    EXPECT_EQ( parser_state.ribbon.at(2).m_type, Token_t::literal_string);
    EXPECT_EQ( parser_state.ribbon.at(3).m_type, Token_t::parenthesis_close);

    // parse
    Optional<Slot*> function_out = language->parse_function_call();

    // check
    EXPECT_TRUE(function_out.valid());
    EXPECT_TRUE(function_out->node->type() == NodeType_FUNCTION);
}

TEST_F(Language_parse_function_call, operator_add)
{
    // prepare parser
    std::string buffer{"42+42"};
    auto* language = app.get_language();
    Nodlang::ParserState& parser_state = language->parser_state;
    parser_state.clear();
    parser_state.set_source_buffer(buffer.data(), buffer.length());
    parser_state.graph = app.get_graph();

    // tokenize
    language->tokenize(buffer);

    // check
    EXPECT_EQ( parser_state.ribbon.size(), 3);
    EXPECT_EQ( parser_state.ribbon.at(0).m_type, Token_t::literal_int);
    EXPECT_EQ( parser_state.ribbon.at(1).m_type, Token_t::operator_);
    EXPECT_EQ( parser_state.ribbon.at(2).m_type, Token_t::literal_int);

    // parse
    Optional<Slot*> result = language->parse_expression();

    // check
    EXPECT_TRUE(result.valid());
    EXPECT_TRUE(result->node->type() == NodeType_OPERATOR);
}