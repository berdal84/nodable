#include "core/language/Nodlang.h"
#include "../fixtures/core.h"
#include <gtest/gtest.h>

using namespace ndbl;

typedef ::testing::Core Language_parse_function_call;

TEST_F(Language_parse_function_call, dna_to_protein)
{
    // tokenize
    lang_tokenize(language(), "dna_to_protein(\"GATACA\")");

    // check
    Token_Ribbon& ribbon = language().ribbon;
    EXPECT_EQ(ribbon.size(), 4);
    EXPECT_EQ(ribbon.at(0).type, Token_Type_identifier);
    EXPECT_EQ(ribbon.at(1).type, Token_Type_parenthesis_open);
    EXPECT_EQ(ribbon.at(2).type, Token_Type_literal_string);
    EXPECT_EQ(ribbon.at(3).type, Token_Type_parenthesis_close);

    // parse
    Node_Slot* function_out = lang_parse_function_call( language(), graph_root_scope(app.graph) );

    // check
    EXPECT_TRUE(function_out!= nullptr);
    EXPECT_TRUE(function_out->node->type == Node_Type_FUNCTION);
}

TEST_F(Language_parse_function_call, operator_add)
{
    // tokenize
    lang_tokenize(language(), "42+42");

    // check
    Token_Ribbon& ribbon = language().ribbon;
    EXPECT_EQ(ribbon.size(), 3);
    EXPECT_EQ(ribbon.at(0).type, Token_Type_literal_int);
    EXPECT_EQ(ribbon.at(1).type, Token_Type_operator);
    EXPECT_EQ(ribbon.at(2).type, Token_Type_literal_int);

    // parse
    Node_Slot* result = lang_parse_expression( language(), graph_root_scope(app.graph) );

    // check
    EXPECT_TRUE(result != nullptr );
    EXPECT_TRUE(result->node->type == Node_Type_OPERATOR);
}