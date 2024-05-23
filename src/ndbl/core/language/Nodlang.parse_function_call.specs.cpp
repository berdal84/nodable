#include "../fixtures/core.h"
#include <gtest/gtest.h>
#include "tools/core/log.h"
#include "ndbl/core/InvokableComponent.h"

using namespace ndbl;

typedef ::testing::Core parse_function_call;

TEST_F(parse_function_call, dna_to_protein)
{
    // prepare parser
    std::string buffer{"dna_to_protein(\"GATACA\")"};
    Nodlang::ParserState& parser_state = app.language.parser_state;
    parser_state.clear();
    parser_state.set_source_buffer(buffer.data(), buffer.length());
    parser_state.graph = &app.graph;

    // tokenize
    app.language.tokenize(buffer);

    // check
    EXPECT_EQ( parser_state.ribbon.size(), 4);
    EXPECT_EQ( parser_state.ribbon.at(0).m_type, Token_t::identifier);
    EXPECT_EQ( parser_state.ribbon.at(1).m_type, Token_t::parenthesis_open);
    EXPECT_EQ( parser_state.ribbon.at(2).m_type, Token_t::literal_string);
    EXPECT_EQ( parser_state.ribbon.at(3).m_type, Token_t::parenthesis_close);

    // parse
    Slot* function_out = app.language.parse_function_call();

    // check
    EXPECT_TRUE(function_out);
    Node* invokable_node = function_out->get_node();
    EXPECT_TRUE(invokable_node->has_component<InvokableComponent>());
    EXPECT_TRUE(invokable_node->get_component<InvokableComponent>()->has_function()); // should not be abstract
}

TEST_F(parse_function_call, operator_add)
{
    // prepare parser
    std::string buffer{"42+42"};
    Nodlang::ParserState& parser_state = app.language.parser_state;
    parser_state.clear();
    parser_state.set_source_buffer(buffer.data(), buffer.length());
    parser_state.graph = &app.graph;

    // tokenize
    app.language.tokenize(buffer);

    // check
    EXPECT_EQ( parser_state.ribbon.size(), 3);
    EXPECT_EQ( parser_state.ribbon.at(0).m_type, Token_t::literal_int);
    EXPECT_EQ( parser_state.ribbon.at(1).m_type, Token_t::operator_);
    EXPECT_EQ( parser_state.ribbon.at(2).m_type, Token_t::literal_int);

    // parse
    Slot* result = app.language.parse_expression();

    // check
    EXPECT_TRUE(result);
    Node* invokable_node = result->get_node();
    EXPECT_TRUE(invokable_node->has_component<InvokableComponent>());
    EXPECT_TRUE(invokable_node->get_component<InvokableComponent>()->has_function()); // should not be abstract
}