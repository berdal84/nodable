#include "test/fixtures/nodable_headless_test.h"
#include <gtest/gtest.h>
#include <iostream>

typedef ::testing::Nodable_Headless_Test DISABLED_Language_parse_and_serialize;
typedef ::testing::Nodable_Headless_Test Language_parse_and_serialize;
typedef ::testing::Nodable_Headless_Test Language_parse_token;
typedef ::testing::Nodable_Headless_Test Language_tokenize;
typedef ::testing::Nodable_Headless_Test Language_basics;
typedef ::testing::Nodable_Headless_Test Language_parse_function_call;

using namespace ndbl;

TEST_F(Language_basics, can_get_add_operator_with_short_identifier )
{
    EXPECT_TRUE(lang_find_operator(language(), {"+", Operator_Type::Binary}));
    EXPECT_TRUE(lang_find_operator(language(), {"-", Operator_Type::Unary}));
}
TEST_F(Language_basics, token_t_to_type)
{
    EXPECT_EQ(lang_get_type(language(), Token_Type_keyword_bool)  , type_get<bool>());
    EXPECT_EQ(lang_get_type(language(), Token_Type_keyword_double), type_get<double>() );
    EXPECT_EQ(lang_get_type(language(), Token_Type_keyword_i16)   , type_get<i16_t>() );
    EXPECT_EQ(lang_get_type(language(), Token_Type_keyword_int)   , type_get<i32_t>() );
    EXPECT_EQ(lang_get_type(language(), Token_Type_keyword_string), type_get<bdc::String>() );
    EXPECT_EQ(lang_get_type(language(), Token_Type_keyword_any)   , type_get<any>() );

    EXPECT_EQ(lang_get_type(language(), Token_Type_literal_bool)    , nullptr);
    EXPECT_EQ(lang_get_type(language(), Token_Type_literal_double)  , nullptr);
    EXPECT_EQ(lang_get_type(language(), Token_Type_literal_int)     , nullptr);
    EXPECT_EQ(lang_get_type(language(), Token_Type_literal_string)  , nullptr);
    EXPECT_EQ(lang_get_type(language(), Token_Type_literal_any)     , nullptr);
}

TEST_F(Language_basics, type_to_string)
{
    EXPECT_EQ(lang_serialize_type(language(), type_get<bool>())        , "bool" );
    EXPECT_EQ(lang_serialize_type(language(), type_get<double>())      , "double" );
    EXPECT_EQ(lang_serialize_type(language(), type_get<i16_t>())       , "i16" );
    EXPECT_EQ(lang_serialize_type(language(), type_get<int>())         , "int" );
    EXPECT_EQ(lang_serialize_type(language(), type_get<i32_t>())       , "int" );
    EXPECT_EQ(lang_serialize_type(language(), type_get<bdc::String>()) , "string" );
    EXPECT_EQ(lang_serialize_type(language(), type_get<any>())         , "any" );
}



///////////////////////// Atomic expressions ///////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_token, atomic_expression_if)
{
    bdc::String buffer{"if"};
    Token token = lang_parse_token(language(), buffer);
    EXPECT_EQ(token.type, Token_Type_keyword_if);
    EXPECT_EQ(token.view(), "if");
}

TEST_F(Language_parse_token, atomic_expression_else)
{
    bdc::String buffer{"else"};
    Token token = lang_parse_token(language(), buffer);
    EXPECT_EQ(token.type, Token_Type_keyword_else);
    EXPECT_EQ(token.view(), "else");
}

TEST_F(Language_parse_token, atomic_expression_for)
{
    bdc::String buffer{"for"};
    Token token = lang_parse_token(language(), buffer);
    EXPECT_EQ(token.type, Token_Type_keyword_for);
    EXPECT_EQ(token.view(), "for");
}

TEST_F(Language_parse_token, atomic_expression_bool_true)
{
    bdc::String buffer{"true"};
    Token token = lang_parse_token(language(), buffer);
    EXPECT_EQ(token.type, Token_Type_literal_bool);
    EXPECT_EQ(token.view(), "true");
}

TEST_F(Language_parse_token, atomic_expression_bool_false)
{
    bdc::String buffer{"false"};
    Token token = lang_parse_token(language(), buffer);
    EXPECT_EQ(token.type, Token_Type_literal_bool);
    EXPECT_EQ(token.view(), "false");
}

TEST_F(Language_parse_token, atomic_expression_int_5)
{
    bdc::String buffer{"5"};
    Token token = lang_parse_token(language(), buffer);
    EXPECT_EQ(token.type, Token_Type_literal_int);
}

TEST_F(Language_parse_token, atomic_expression_double_5_0)
{
    bdc::String buffer{"5.0"};
    Token token = lang_parse_token(language(), buffer);
    EXPECT_EQ(token.type, Token_Type_literal_double);
    EXPECT_EQ(token.view(), "5.0");
}

TEST_F(Language_parse_token, atomic_expression_double_5_0001)
{
    bdc::String buffer{"5.0001"};
    Token token = lang_parse_token(language(), buffer);
    EXPECT_EQ(token.type, Token_Type_literal_double);
    EXPECT_EQ(token.view(), "5.0001");
}

TEST_F(Language_parse_token, atomic_expression_string)
{
    bdc::String buffer{"\"Hello\""};
    Token token = lang_parse_token(language(), buffer);
    EXPECT_EQ(token.type, Token_Type_literal_string);
    EXPECT_EQ(token.view(), "\"Hello\"");
}



//////////////////////////// Identifiers ///////////////////////////////////////////////////////////////////////////////

TEST_F(Language_tokenize, identifiers_can_start_by_a_keyword)
{
    bdc::String code = "int if_myvar_includes_a_keyword;";
    lang_tokenize(language(), code);
    log_ribbon();
    const Token& token = language().ribbon[1];
    EXPECT_EQ(token.word_view(), "if_myvar_includes_a_keyword");
    EXPECT_EQ(token.type, Token_Type_identifier);
}

//////////////////////////// Prefix / Suffix ///////////////////////////////////////////////////////////////////////////

TEST_F(Language_tokenize, identifiers_should_not_have_prefix_or_suffix)
{
    bdc::String code{"int my_var ;"};
    lang_tokenize(language(), code);
    log_ribbon();
    const Token& token = language().ribbon[1];
    EXPECT_EQ(token.word_view()     , "my_var");
    EXPECT_EQ(token.prefix_view()   , "");
    EXPECT_EQ(token.suffix_view()   , "");
}

TEST_F(Language_tokenize, operator_suffix_and_prefix)
{
    bdc::String code{"int my_var = 42"};
    lang_tokenize(language(), code);
    log_ribbon();
    const Token& token = language().ribbon[2];
    EXPECT_EQ(token.view()          , " = ");
    EXPECT_EQ(token.prefix_view()   , " ");
    EXPECT_EQ(token.suffix_view()   , " ");
}

TEST_F(Language_tokenize, operator_suffix)
{
    bdc::String code = "int my_var= 42";
    lang_tokenize(language(), code);
    log_ribbon();
    const Token& token = language().ribbon[2];
    EXPECT_EQ(token.view()          , "= ");
    EXPECT_EQ(token.prefix_view()   , "");
    EXPECT_EQ(token.suffix_view()   , " ");
}

TEST_F(Language_tokenize, operator_prefix)
{
    bdc::String code = "int my_var =42";
    lang_tokenize(language(), code);
    log_ribbon();
    const Token& token = language().ribbon[2];
    EXPECT_EQ(token.view()          , " =");
    EXPECT_EQ(token.prefix_view()   , " " );
    EXPECT_EQ(token.suffix_view()   , ""  );
}


TEST_F(Language_tokenize, add_pow2of2_and_integer )
{
    bdc::String code = "pow(2,2) + 1";
    lang_tokenize(language(), code);
    log_ribbon();
    EXPECT_EQ(language().ribbon[2].view(), "2");
    EXPECT_EQ(language().ribbon[3].view(), ",");
    EXPECT_EQ(language().ribbon[4].view(), "2");
    EXPECT_EQ(language().ribbon[5].view(), ")"); // parser should not add a " " prefix after ")"
    EXPECT_EQ(language().ribbon[6].view(), " + ");
    EXPECT_EQ(language().ribbon[7].view(), "1");

}

TEST_F(Language_tokenize, return_integer )
{
    bdc::String code = "return 42";
    lang_tokenize(language(), code);
    EXPECT_EQ(language().ribbon[0].word_view(), "return");
    EXPECT_EQ(language().ribbon[1].word_view(), "42");

}

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


TEST_F(Language_parse_and_serialize, decl_var_and_assign_string)
{
    String code   = R"(string s = "coucou";)";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, decl_var_and_assign_double)
{
    String code   = "double d = 15.0;";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, decl_var_and_assign_int)
{
    String code   = "int s = 10;";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, decl_var_and_assign_bool)
{
    String code   = "bool b = true;";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

/////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, While_loop )
{
    String code =
        "int i = 0;"
        "while(i < 10){"
        "   i = i+1;"
        "}";
    String result = parse_and_serialize(code);
    EXPECT_EQ( result, code);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, Conditional_Structures_IF )
{
    String code =
        "double bob   = 10;"
        "double alice = 10;"
        "if(bob>alice){"
        "   string message = \"Bob is better than Alice.\";"
        "}";
    String result = parse_and_serialize(code);
    EXPECT_EQ( result, code);
}

TEST_F(Language_parse_and_serialize, Conditional_Structures_IF_ELSE )
{
    String code =
        "double bob   = 10;"
        "double alice = 11;"
        "string message;"
        "if(bob<alice){"
        "   message= \"Alice is the best.\";"
        "}else{"
        "   message= \"Alice is not the best.\";"
        "}";
    String result = parse_and_serialize(code);
    EXPECT_EQ( result, code);
}

TEST_F(Language_parse_and_serialize, Conditional_Structures_IF_ELSE_IF )
{
    String code =
        "double bob   = 10;"
        "double alice = 10;"
        "string message;"
        "if(bob>alice){"
        "   message= \"Bob is greater than Alice.\";"
        "} else if(bob<alice){"
        "   message= \"Bob is lower than Alice.\";"
        "} else {"
        "   message= \"Bob and Alice are is.\";"
        "}";
    String result = parse_and_serialize(code);
    EXPECT_EQ( result, code);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, parse_serialize_with_undeclared_variables )
{
    String code   = "double a = b + c * r - z;";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, parse_serialize_with_undeclared_variables_in_conditional )
{
    String code   = "if(a==b){}";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

/////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, parse_serialize_empty_code )
{
    String result = parse_and_serialize("");
    EXPECT_EQ( result, "" );
}

TEST_F(Language_parse_and_serialize, parse_serialize_empty_code_with_space )
{
    String result = parse_and_serialize(" ");
    EXPECT_EQ( result, " " );
}

TEST_F(Language_parse_and_serialize, parse_serialize_single_line_code_with_a_comment_before )
{
    String code =
        "// comment\n"
        "int a = 42;";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, parse_serialize_single_code_line_with_two_sigle_line_comments_and_a_space )
{
    String code =
        "// first line\n"
        "// second line\n"
        "\n"
        "int a = 42;";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, parse_serialize_binary_expression_with_funtion )
{
    String code   = "int i = pow(2,2) + 1";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code); // should not be "int i = (pow(2,2))+ 1"
}

/////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, parse_serialize_variable_declaration )
{
    String code   = "int i = 42;";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

/////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, parse_serialize_variable_referenced )
{
    String code   = "int i = 42; int j = i;";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, parse_serialize_variable_referenced2 )
{
    String code   = "int i = 42; i;";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

/////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, parse_serialize_empty_scope )
{
    String result = parse_and_serialize("{}");
    EXPECT_EQ(result, "{}");
}

TEST_F(Language_parse_and_serialize, parse_serialize_empty_scope_with_spaces )
{
    String result = parse_and_serialize("{ }");
    EXPECT_EQ(result, "{ }");
}

TEST_F(Language_parse_and_serialize, parse_serialize_empty_scope_with_spaces_after )
{
    String result = parse_and_serialize("{} ");
    EXPECT_EQ(result, "{} ");
}

TEST_F(Language_parse_and_serialize, parse_serialize_empty_scope_with_spaces_before )
{
    String result = parse_and_serialize(" {}");
    EXPECT_EQ(result, " {}");
}

TEST_F(Language_parse_and_serialize, parse_serialize_empty_scope_with_spaces_before_and_after )
{
    String result = parse_and_serialize(" {} ");
    EXPECT_EQ(result, " {} ");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, partial_for1)
{
    String code = "for();";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, partial_for2)
{
    String code   = "for(;);";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, partial_for3)
{
    String code   = "for(;;);";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, partial_if1)
{
    String code    = "if();";
    String result  = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, partial_if2)
{
    String code    = "if();else;";
    String result  = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, partial_if3)
{
    String code    = "if()else;";
    String result  = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize, partial_if4)
{
    String code    = "if()else";
    String result  = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize, partial_while1)
{
    String code   = "while();";
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(Language_parse_and_serialize , exemple_arithmetic)
{
    String code   = load_file("examples/arithmetic.cpp");
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

TEST_F(Language_parse_and_serialize , example_for_loop)
{
    String code   = load_file("examples/for-loop.cpp");
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

// TODO: handle missing spaces around in var refs
TEST_F(DISABLED_Language_parse_and_serialize , example_if_else)
{
    String code   = load_file("examples/if-else.cpp");
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}

// TODO: handle missing spaces around in var refs
TEST_F(DISABLED_Language_parse_and_serialize , exemple_multi_instructions)
{
    String code   = load_file("examples/multi-instructions.cpp");
    String result = parse_and_serialize(code);
    EXPECT_EQ(result, code);
}