#include "Parser.h"

#include <limits>
#include <cctype> // isdigit, isalpha, and isalnum.

#include "bdc/String.hpp"
#include "bdc/String_Builder.hpp"
#include "bdc/Types.hpp"
#include "Asserts.h"
#include "Constants.h"
#include "Format.h"
#include "Graph.h"
#include "Hash.h"
#include "Log.h"
#include "Node_Property.h"
#include "Node_Slot.h"
#include "Node.h"
#include "reflection/Operator.h"
#include "Scope.h"
#include "Token_Type.h"

namespace ndbl
{
using namespace bdc;

//---------------------------------------------------------------------------------------------------------------------------
//
//                                  LANGUAGE DEFINITION
//
//---------------------------------------------------------------------------------------------------------------------------

static Language_Definition* g_langdef = nullptr;

Language_Definition& langdef_init()
{
    ASSERT(g_langdef == nullptr);

    Language_Definition* langdef = bdc::memory_new<Language_Definition>();

    // A.1. Define the ctx
    //-------------------------
    array_init(langdef->chars);

    array_append(langdef->chars, {
        { '(',  Token_Type_parenthesis_open},
        { ')',  Token_Type_parenthesis_close},
        { '{',  Token_Type_scope_begin},
        { '}',  Token_Type_scope_end},
        { '\n', Token_Type_end_of_line},
        { '\t', Token_Type_tab},
        { ' ',  Token_Type_space},
        { ';',  Token_Type_end_of_instruction},
        { ',',  Token_Type_list_separator}
    });

    array_init(langdef->keywords);
    array_append(langdef->keywords, {
        { "if",       Token_Type_keyword_if },
        { "for",      Token_Type_keyword_for },
        { "while",    Token_Type_keyword_while },
        { "else",     Token_Type_keyword_else },
        { "true",     Token_Type_literal_bool },
        { "false",    Token_Type_literal_bool },
        { "operator", Token_Type_keyword_operator },
        { "return",   Token_Type_keyword_return }
    });

    array_init(langdef->types);
    array_append(langdef->types, {
        // TODO: instead of using type_get<T>(), I should use a more datadriven option,
        //       I should be able to do type_get(Token_Type_keyword_bool) for example,
        //       Or with an indirection level  type_get( token_type_keyword_to_type(Token_Type_keyword_bool) )  
        { "bool",   Token_Type_keyword_bool,   type_get<bool>()},
        { "string", Token_Type_keyword_string, type_get<bdc::String>()},
        { "double", Token_Type_keyword_double, type_get<double>()},
        { "i16",    Token_Type_keyword_i16,    type_get<i16_t>()},
        { "int",    Token_Type_keyword_int,    type_get<i32_t>()},
        { "any",    Token_Type_keyword_any,    type_get<any>()},
        // we don't really want to parse/serialize that
        // { "unknown",Token_t::keyword_unknown,type_get<unknown>()},
    });

    array_init(langdef->operators);
    array_append(langdef->operators, {
        {"-",   Operator_Type::Unary,   5},
        {"!",   Operator_Type::Unary,   5},
        {"/",   Operator_Type::Binary, 20},
        {"*",   Operator_Type::Binary, 20},
        {"+",   Operator_Type::Binary, 10},
        {"-",   Operator_Type::Binary, 10},
        {"||",  Operator_Type::Binary, 10},
        {"&&",  Operator_Type::Binary, 10},
        {">=",  Operator_Type::Binary, 10},
        {"<=",  Operator_Type::Binary, 10},
        {"=>",  Operator_Type::Binary, 10},
        {"==",  Operator_Type::Binary, 10},
        {"<=>", Operator_Type::Binary, 10},
        {"!=",  Operator_Type::Binary, 10},
        {">",   Operator_Type::Binary, 10},
        {"<",   Operator_Type::Binary, 10},
        {"=",   Operator_Type::Binary,  0},
        {"+=",  Operator_Type::Binary,  0},
        {"-=",  Operator_Type::Binary,  0},
        {"/=",  Operator_Type::Binary,  0},
        {"*=",  Operator_Type::Binary,  0}
    });

    // A.2. Create indexes
    //---------------------
    for( auto& character : langdef->chars )
    {
        hashmap_add(langdef->token_type_by_single_char, character.id        , character.token_type);
        hashmap_add(langdef->single_char_by_keyword   , character.token_type, character.id        );
    }

    for( auto& keyword : langdef->keywords )
    {
        hashmap_add(langdef->token_type_by_keyword, string_hash(keyword.id).hash, keyword.token_type);
        hashmap_add(langdef->keyword_by_token_type, keyword.token_type          , keyword.id        );
    }

    for( auto& type : langdef->types )
    {
        hashmap_add(langdef->keyword_by_token_type         , type.token_type                       , type.id               );
        hashmap_add(langdef->keyword_by_type_id            , type.type_descriptor->id.hash_code()  , type.id               );
        hashmap_add(langdef->token_type_by_keyword         , string_hash(type.id).hash             , type.token_type       );
        hashmap_add(langdef->token_type_by_type_id         , type.type_descriptor->id.hash_code()  , type.token_type       );
        hashmap_add(langdef->type_descriptor_by_token_type , type.token_type                       , type.type_descriptor  );
    }

    g_langdef = langdef;

    return *langdef;
}

bool langdef_is_initialized()
{
    return g_langdef != nullptr;
}

Language_Definition& langdef()
{
    VERIFY(g_langdef, "No ctx found, did you call init_language?");
    return *g_langdef;
}

void langdef_shutdown()
{
    ASSERT(g_langdef != nullptr);

    array_release(g_langdef->chars);
    array_release(g_langdef->keywords);
    array_release(g_langdef->types);
    array_release(g_langdef->operators);
    array_release(g_langdef->operators);

    bdc::memory_delete(g_langdef);
    g_langdef = nullptr;
}

const Operator* langdef_find_operator(const Language_Definition& langdef, const Operator& op)
{
    // TODO: This function is very slow, it iterates over all operators each call (worse case).
    //       I should index operators, implement a hash function for it, and use bdc::Hash_Map.

    for(auto& each : langdef.operators)
    {
        if( each == op)
            return &each;
    }

    return nullptr;
}

bool langdef_is_operator(const Language_Definition& langdef, const Type_Descriptor* type)
{
    switch ( type->function.args.size )
    {
        case 1:     return langdef_find_operator( langdef, Operator{ type->name, Operator_Type::Unary } );
        case 2:     return langdef_find_operator( langdef, Operator{ type->name, Operator_Type::Binary } );
        default:    return false;
    }
}

int langdef_get_precedence(const Language_Definition& langdef, const Type_Descriptor* _func_type)
{
    if (!_func_type)
        return std::numeric_limits<int>::min(); // default

    Operator expected_operator{ _func_type->name, static_cast<Operator_Type>(_func_type->function.args.size) };

    if (const Operator* found_operator = langdef_find_operator(langdef, expected_operator))
        return found_operator->precedence;
    return std::numeric_limits<int>::max();
}

const Type_Descriptor* langdef_get_type_descriptor_from_token_type(const Language_Definition& langdef, Token_Type _token)
{
    if ( auto found = hashmap_find( langdef.type_descriptor_by_token_type, _token) )
        return found.value;
    return nullptr;
}

Token_Type langdef_type_descriptor_to_token_type_literal(const Language_Definition& langdef, const Type_Descriptor *type)
{
    //
    // TODO: this should be in a Hash_Map right?
    //

    if (type == type_get<double>() )
        return Token_Type_literal_double;
    if (type == type_get<i16_t>() )
        return Token_Type_literal_int;
    if (type == type_get<int>() )
        return Token_Type_literal_int;
    if (type == type_get<bool>() )
        return Token_Type_literal_bool;
    if (type == type_get<bdc::String>() )
        return Token_Type_literal_string;
    if (type == type_get<any>() )
        return Token_Type_literal_any;
    return Token_Type_literal_unknown;
}

//---------------------------------------------------------------------------------------------------------------------------
//
//                                  PARSING
//
//---------------------------------------------------------------------------------------------------------------------------

void parser_init(Parser_Context& ctx, Graph* out_graph, bdc::String in_text )
{
    ctx.langdef     = &langdef(); // langdef if unique for now...
    ctx.strict_mode = false;      // we want to get a graph even if the code does not compiles, like a text editor with some source code.
    parser_reset(ctx, out_graph, in_text);
}

void parser_deinit(Parser_Context& ctx)
{
  // ..
}

void parser_reset(Parser_Context& ctx, Graph* out_graph, bdc::String in_text )
{
    ctx.in_text   = in_text;
    ctx.out_graph = out_graph;

    ctx.tokens.clear();
    ctx.pristine_tokens.clear();
    ctx.global_token.replace_buffer( ctx.in_text, true ); // wraps all

    while(!ctx.transaction.empty())
        ctx.transaction.pop();
    ctx.cursor = 0;

    if( ctx.out_graph ) // not all parse_xxx procedures requires it, but if it is set we must reset it
    {
        graph_reset( ctx.out_graph );
    }
}

bool parse_graph(Parser_Context& ctx)
{
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing ...\n%s\n", ctx.in_text.c_str() );
    VERIFY(ctx.out_graph != nullptr, "A Graph is expected");

    //
    // Optimize the Token_Ribbon
    // - make sure not token is a space/end line/etc..
    // - attach the text of the removed tomes
    //

    NDBL_LOG(Verbosity_Diagnostic, "Parser", "Tokenization ...\n");
    ASSERT(ctx.in_text.data != nullptr);

    size_t ignored_chars_count = 0;
    ctx.pristine_tokens = ctx.tokens;
    ctx.tokens.clear();
    for(size_t i = 0; i < ctx.pristine_tokens.size(); ++i )
    {
        Token& current_token = ctx.pristine_tokens[i];

        // If the token is not important in terms of semantic, we'll put in as prefix/suffix on a an other Token.
        switch(current_token.type)
        {
            case Token_Type_space:
            case Token_Type_tab:
            case Token_Type_end_of_line:
            case Token_Type_comment:
            case Token_Type_multiline_comment:
            {
                if ( ctx.tokens.empty() )
                {
                    ctx.global_token.ltrim_word( current_token.size() );
                    continue;
                }

                ignored_chars_count += current_token.size();
                continue;
            }
        }

        if ( i > 0 && ignored_chars_count > 0 )
        {
            Token& previous_token = ctx.tokens.back();
            switch (previous_token.type)
            {
                case Token_Type_identifier:        // identifiers must stay clean because they are reused
                case Token_Type_parenthesis_open:  // ")" are lost when creating AST
                case Token_Type_parenthesis_close: // "("
                {
                    if ( current_token )
                    {
                        current_token.lextend_prefix(ignored_chars_count);
                    }
                    break;
                }
                default:
                {
                    previous_token.rextend_suffix(ignored_chars_count);
                    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "      \"%s\" (update) \n", previous_token.view().c_str() );
                }
            }

            ignored_chars_count = 0;
        }

        ctx.tokens.push_back(current_token);
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "%4llu) \"%s\" \n", current_token.index, current_token.view().c_str() );
    }

    // Append remaining ignored chars to the ribbon's suffix
    if ( ignored_chars_count )
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Found ignored chars after tokenize, adding to the tokens suffix...\n");
        Token& tok = ctx.global_token;
        tok.rtrim_word( ignored_chars_count );
    }

    NDBL_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Token_Ribbon optimized.\n%s\n", parser_to_string(ctx).c_str() );

    //
    // Build the graph
    //    

    parser_start_transaction(ctx);

    Scope* scope = graph_root_scope(ctx.out_graph);

    // Parse main code block
    Node* block_last_node = parse_code_block( ctx, scope, scope->node->flow_enter() );

    // To preserve any ignored characters stored in the global token
    // we put the prefix and suffix in resp. token_begin and end.
    Token& tok = ctx.global_token;

    if (tok.prefix_size) scope->token_begin.prefix_push_front( tok.prefix_view() );
    if (tok.suffix_size) scope->token_end.suffix_push_back( tok.suffix_view() );

    if ( parser_can_eat(ctx) )
    {
        parser_rollback(ctx);
        graph_reset(ctx.out_graph);
        ctx.out_graph->signal_is_complete.emit();
        NDBL_LOG(Verbosity_Warning, "Parser", "Some token remains after getting an empty code block\n");
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " End of token ribbon expected\n");
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "%s", Format::title("Token_Ribbon").c_str());
        for (const Token& each_token : ctx.tokens )
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "token idx %i: %s\n", each_token.index, each_token.json().c_str());
        }
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "%s", Format::title("Token_Ribbon end").c_str());
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Failed to parse from token at index %llu (tokens.size: %llu) and above.\n", parser_peek(ctx).index, ctx.tokens.size());
        NDBL_LOG(Verbosity_Error, "Parser", "Unable to parse all the tokens\n");
        
        return false;
    }
    
    if ( block_last_node == nullptr )
    {
        NDBL_LOG(Verbosity_Warning, "Parser", "Program main block is empty\n");
    }

    parser_commit(ctx);
    ctx.out_graph->signal_is_complete.emit();

    NDBL_LOG(Verbosity_Message, "Parser", "Parse program [OK]\n");

    return true;
}

bool parse_bool_or(const Parser_Context& ctx, bdc::String& buffer, bool default_value)
{
    Token token = parse_token( ctx, buffer);
    if (token.type == Token_Type_literal_bool )
        return token.word_view() == "true";
    return default_value;
}

double parse_double_or(const Parser_Context& ctx, bdc::String& buffer, double default_value)
{
    Token token  = parse_token( ctx, buffer);

    if (token.type == Token_Type_literal_double )
    {
        return std::stod( token.word_view().c_str() );
    }

    return default_value;
}

int parse_int_or(const Parser_Context& ctx, bdc::String& buffer, int default_value)
{
    Token token  = parse_token( ctx, buffer);

    if (token.type == Token_Type_literal_int )
    {
        i64_t l = atoll(token.word_view().c_str());
        int n = std::clamp(l, (i64_t)std::numeric_limits<int>::min() , (i64_t)std::numeric_limits<int>::max());
        if( n > (int)l )
        {
            NDBL_LOG( Verbosity_Warning, "Parser", "Parsing a too large integer for 32bits!\n");
        }
        return n;
    }
    return default_value;
}

Node_Slot* parse_token(const Parser_Context& ctx, Scope* parent_scope, const Token& _token)
{
    if (_token.type == Token_Type_identifier)
    {
        bdc::String identifier = _token.word_view();
        if( Node* existing_node = scope_find_variable(parent_scope, identifier) )
        {
            return existing_node->component.variable.ref_out;
        }

        if ( ctx.strict_mode )
        {
            NDBL_LOG(Verbosity_Error,  "Parser", "%s is not declared (strict mode) \n", _token.word_view().c_str() );
            return nullptr;
        }

        // Insert a variable reference Node pointing to nothing.
        NDBL_LOG(Verbosity_Warning,  "Parser", "%s is not declared, abstract graph can be generated but compilation will fail.\n",
                    _token.word_view().c_str() );
        Node* ref = graph_create_variable_ref( ctx.out_graph, parent_scope );
        ref->value->token = _token;
        return ref->value_out();
        
    }

    Node* literal = nullptr;

    switch (_token.type)
    {
        case Token_Type_literal_bool:   literal = graph_create_literal<bool>(ctx.out_graph, parent_scope );        break;
        case Token_Type_literal_int:    literal = graph_create_literal<i32_t>( ctx.out_graph, parent_scope );       break;
        case Token_Type_literal_double: literal = graph_create_literal<double>( ctx.out_graph, parent_scope );      break;
        case Token_Type_literal_string: literal = graph_create_literal<bdc::String>( ctx.out_graph, parent_scope ); break;
        default:
            break; // we don't want to throw
    }

    if ( literal )
    {
        NDBL_DEBUG_LOG(
            Verbosity_Diagnostic, "Parser", NDBL_OK " Token %s converted to a Literal %s\n",
            _token.word_view().c_str(),
            literal->value->type->name.c_str()
        );
        literal->value->token = _token;
        return literal->value_out();
    }

    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Unable to run token_to_slot with token %s!\n", _token.word_view().c_str());
    return nullptr;
}

Node_Slot* parse_binary_operation_expression(Parser_Context& ctx, Scope* parent_scope, u8_t _precedence, Node_Slot* _left)
{
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing binary expression ...\n");
    ASSERT(_left != nullptr);

    if (!parser_can_eat(ctx, 2))
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Not enough tokens\n");
        return nullptr;
    }

    parser_start_transaction(ctx);
    const Token operator_token = parser_eat(ctx);
    const Token operand_token  = parser_peek(ctx);

    // Structure check
    const bool isValid = operator_token.type == Token_Type_operator &&
                        operand_token.type != Token_Type_operator;

    if (!isValid)
    {
        parser_rollback(ctx);
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Unexpected tokens\n");
        return nullptr;
    }

    const Operator *ope = langdef_find_operator(*ctx.langdef, Operator{ operator_token.word_view(), Operator_Type::Binary} );
    if (ope == nullptr)
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Operator %s not found\n", operator_token.word_view().c_str());
        parser_rollback(ctx);
        return nullptr;
    }

    // Precedence check
    if (ope->precedence <= _precedence && _precedence > 0)
    {// always update the first operation if they have the same precedence or less.
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Has lower precedence\n");
        parser_rollback(ctx);
        return nullptr;
    }

    // Parse right expression
    if ( Node_Slot* right = parse_expression(ctx, parent_scope, ope->precedence) )
    {
        // Create a function signature according to ltype, rtype and operator word
        Type_Descriptor type;
        type_init<any(any, any)>(&type);
        type.name = ope->identifier;
        type.function.args[0].type = _left->property->type;
        type.function.args[1].type = right->property->type;

        Node* binary_op_node = graph_create_operator( ctx.out_graph, &type, _left->node->scope );

        Node::Invokable_Component& binary_op = binary_op_node->component.invokable;

        binary_op.identifier_token = operator_token;
        binary_op.lvalue_in()->property->token.type = _left->property->token.type;
        binary_op.rvalue_in()->property->token.type = right->property->token.type;

        graph_connect_or_merge(_left, binary_op.lvalue_in());
        graph_connect_or_merge(right, binary_op.rvalue_in() );

        parser_commit(ctx);
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Binary expression parsed:\n%s\n", parser_to_string(ctx).c_str());
        return binary_op_node->value_out();
    }

    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Right expression is null\n");
    parser_rollback(ctx);
    return nullptr;
}

Node_Slot* parse_unary_operation_expression(Parser_Context& ctx, Scope* parent_scope, u8_t _precedence)
{
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "parseUnaryOperationExpression...\n");

    if (!parser_can_eat(ctx, 2))
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Not enough tokens\n");
        return nullptr;
    }

    parser_start_transaction(ctx);
    Token operator_token = parser_eat(ctx);

    // Check if we get an operator first
    if (operator_token.type != Token_Type_operator)
    {
        parser_rollback(ctx);
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Expecting an operator token first\n");
        return nullptr;
    }

    // Parse expression after the operator
    Node_Slot* out_atomic = parse_atomic_expression( ctx, parent_scope );

    if ( !out_atomic )
    {
        out_atomic = parse_parenthesis_expression( ctx, parent_scope );
    }

    if ( !out_atomic )
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Right expression is null\n");
        parser_rollback(ctx);
        return nullptr;
    }

    // Create a function signature
    Type_Descriptor type;
    type_init<any(any)>(&type);
    type.name = operator_token.word_view();
    type.function.args[0].type = out_atomic->property->type;

    Node* node = graph_create_operator(ctx.out_graph, &type, parent_scope );
    node->component.invokable.identifier_token = operator_token;
    node->component.invokable.lvalue_in()->property->token.type = out_atomic->property->token.type;

    graph_connect_or_merge(out_atomic, node->component.invokable.lvalue_in() );

    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Unary expression parsed:\n%s\n", parser_to_string(ctx).c_str());
    parser_commit(ctx);

    return node->value_out();
}

Node_Slot* parse_atomic_expression(Parser_Context& ctx, Scope* parent_scope)
{
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing atomic expression ... \n");

    if (!parser_can_eat(ctx))
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Not enough tokens\n");
        return nullptr;
    }

    parser_start_transaction(ctx);
    Token token = parser_eat(ctx);

    if (token.type == Token_Type_operator)
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Cannot start with an operator token\n");
        parser_rollback(ctx);
        return nullptr;
    }

    if ( Node_Slot* result = parse_token( ctx, parent_scope, token) )
    {
        parser_commit(ctx);
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Atomic expression parsed:\n%s\n", parser_to_string(ctx).c_str());
        return result;
    }

    parser_rollback(ctx);
    NDBL_DEBUG_LOG(Verbosity_Diagnostic,  "Parser", NDBL_KO " Unable to parse token (%llu)\n", token.index );

    return nullptr;
}

Node_Slot* parse_parenthesis_expression(Parser_Context& ctx, Scope* parent_scope)
{
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "parse parenthesis expr...\n");

    if (!parser_can_eat(ctx))
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " No enough tokens.\n");
        return nullptr;
    }

    parser_start_transaction(ctx);
    Token currentToken = parser_eat(ctx);
    if (currentToken.type != Token_Type_parenthesis_open)
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Open bracket not found.\n");
        parser_rollback(ctx);
        return nullptr;
    }

    Node_Slot* result = parse_expression(ctx, parent_scope);
    if ( result )
    {
        Token token = parser_eat(ctx);
        if (token.type != Token_Type_parenthesis_close)
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "%s \n", parser_to_string(ctx).c_str());
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Parenthesis close expected\n",
                        token.word_view().c_str());
            parser_rollback(ctx);
        }
        else
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Parenthesis expression parsed:\n%s\n", parser_to_string(ctx).c_str());
            parser_commit(ctx);
        }
    }
    else
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " No expression after open parenthesis.\n");
        parser_rollback(ctx);
    }
    return result;
}

Node* parse_expression_block(Parser_Context& ctx, Scope* parent_scope, Node_Slot* flow_out, Node_Slot* value_in)
{
    parser_start_transaction(ctx);

    // Parse an expression
    Node_Slot* value_out = parse_expression(ctx, parent_scope);

    // When expression value_out is a variable that is already part of the code flow,
    // we must create a variable reference
    if ( value_out && value_out->node->type == Node_Type_VARIABLE )
    {
        Node* variable = value_out->node;

        if ( node_is_connected_to_codeflow(variable) ) // in such case, we have to reference the variable, since a given variable can't be twice (be declared twice) in the codeflow
        {
            // create a new variable reference
            Node* ref_node = graph_create_variable_ref( ctx.out_graph, parent_scope );
            node_variable_ref_set_variable( ref_node, variable );
            // substitute value_out by variable reference's value_out
            value_out = ref_node->value_out();
        }
    }

    if ( !parser_can_eat(ctx) )
    {
        // we're passing here if there is no more token, which means we reached the end of file.
        // we allow an expression to end like that.
    }
    else
    {
        // However, in case there are still unparsed tokens, we expect certain type of token, otherwise we reset the result
        switch( parser_peek(ctx).type )
        {
            case Token_Type_end_of_instruction:
            case Token_Type_parenthesis_close:
                NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "End of instruction or parenthesis close: found in next token\n");
                break;
            default:
                NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " End of instruction or parenthesis close expected.\n");
                value_out = nullptr;
        }
    }

    // When expression value_out is null, but an input was provided,
    // we must create an empty instruction if an end_of_instruction token is found
    if (!value_out && value_in )
    {
        if (parser_peek(ctx, Token_Type_end_of_instruction))
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Empty expression found\n");

            Node* empty_instr = graph_create_empty_instruction( ctx.out_graph, parent_scope );
            value_out = empty_instr->value_out();
        }
    }

    // Ensure value_out is defined or rollback transaction
    if ( !value_out )
    {
        parser_rollback(ctx);
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " parse instruction\n");
        return nullptr;
    }

    // Connects value_out to the provided input
    if ( value_in )
    {
        graph_connect( value_out, value_in, Graph_Flag_ALLOW_SIDE_EFFECTS);
    }

    // Add an end_of_instruction token as suffix when needed
    if (Token tok = parser_eat_if(ctx, Token_Type_end_of_instruction))
    {
        value_out->node->suffix = tok;
    }

    // Connects expression flow_in with the provided flow_out
    if ( flow_out != nullptr )
    {
        graph_connect( flow_out, value_out->node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );
    }

    // Validate transaction
    parser_commit(ctx);
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " parse instruction:\n%s\n", parser_to_string(ctx).c_str());

    return value_out->node;
}

Node* parse_empty_block(Parser_Context& ctx, Scope* parent_scope, Node_Slot* flow_out)
{
    if ( parser_peek(ctx, Token_Type_end_of_instruction) )
    {
        Node* node = graph_create_empty_instruction( ctx.out_graph, parent_scope );
        graph_connect( flow_out, node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS);
        return node;
    }
    return nullptr;
}

Node* parse_scoped_block(Parser_Context& ctx, Scope* parent_scope, Node_Slot* flow_out)
{
    ASSERT(parent_scope);
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing scoped block ...\n");

    Token token_begin = parser_eat_if(ctx, Token_Type_scope_begin);
    if ( !token_begin )
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Expecting root_scope begin token\n");
        return nullptr;
    }

    parser_start_transaction(ctx);

    Node* node = graph_create_scope(ctx.out_graph, parent_scope);

    if ( flow_out != nullptr )
        graph_connect( flow_out, node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );


    parse_code_block(ctx, node->internal_scope, node->flow_enter()); // no return check, allows empty scope
    Token token_end = parser_eat_if(ctx, Token_Type_scope_end);

    if ( token_end )
    {
        node->internal_scope->token_begin = token_begin;
        node->internal_scope->token_end = token_end;

        parser_commit(ctx);
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Scoped block parsed:\n%s\n", parser_to_string(ctx).c_str());
        return node;
    }
    else
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Expecting close root_scope token\n");
    }

    graph_find_and_destroy_node(ctx.out_graph, node);
    parser_rollback(ctx);
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Scoped block parsed\n");
    return nullptr;
}

Node* parse_code_block(Parser_Context& ctx, Scope* parent_scope, Node_Slot* flow_out)
{
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing code block...\n" );

    //
    // Parse n atomic code blocks
    //
    parser_start_transaction(ctx);

    Node_Slot* last_node_flow_out  = flow_out;
    bool     block_end_reached = false;
    size_t   block_size        = 0;

    while (parser_can_eat(ctx) && !block_end_reached )
    {
        if ( Node* current_block = parse_atomic_code_block( ctx, parent_scope, last_node_flow_out) )
        {
            last_node_flow_out = current_block->flow_out();
            ++block_size;
        }
        else
        {
            block_end_reached = true;
        }
    }

    if (last_node_flow_out != nullptr && last_node_flow_out != flow_out )
    {
        parser_commit(ctx);
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " parse code block:\n%s\n", parser_to_string(ctx).c_str());
        return last_node_flow_out->node;
    }

    parser_rollback(ctx);
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " parse code block. Block size is %llu\n", block_size );
    return nullptr;
}

Node_Slot* parse_expression(Parser_Context& ctx, Scope* parent_scope, u8_t _precedence, Node_Slot* _left_override)
{
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing expression ...\n");

    /*
        Get the left-handed operand
    */
    Node_Slot* left = _left_override;

    if (!parser_can_eat(ctx))
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Last token reached\n");
        return left;
    }

    if ( !left ) left = parse_parenthesis_expression(ctx, parent_scope);
    if ( !left ) left = parse_unary_operation_expression(ctx, parent_scope, _precedence);
    if ( !left ) left = parse_function_call(ctx, parent_scope);
    if ( !left ) left = parse_variable_declaration(ctx, parent_scope); // nullptr => variable won't be attached on the codeflow, it's a part of an expression..
    if ( !left ) left = parse_atomic_expression(ctx, parent_scope);

    if (!parser_can_eat(ctx))
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Last token reached\n");
        return left;
    }

    if ( !left )
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Left side is null, we return it\n");
        return left;
    }

    /*
        Get the right-handed operand
    */
    Node_Slot* expression_out = parse_binary_operation_expression(ctx, parent_scope, _precedence, left );
    if ( expression_out )
    {
        if (!parser_can_eat(ctx))
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Right side parsed, and last token reached\n");
            return expression_out;
        }
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Right side parsed, continue with a recursive call...\n");
        return parse_expression(ctx, parent_scope, _precedence, expression_out);
    }

    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Returning left side only\n");

    return left;
}

bool parser_tokenize(Parser_Context& ctx)
{
    NDBL_LOG(Verbosity_Diagnostic, "Parser", "Tokenization ...\n");
    ASSERT(ctx.in_text.data != nullptr);

    bdc::String remainder = ctx.in_text;
    size_t ignored_chars_count = 0;

    while ( !remainder.empty() )
    {
        Token  new_token = parse_token( ctx,  remainder );

        if ( !new_token )
        {
            NDBL_LOG(
                Verbosity_Warning, "Parser", 
                NDBL_KO " Unable to tokenize from \"%20s...\" (at char %llu)\n", 
                remainder.c_str(), (u64_t)remainder.data - (u64_t)ctx.in_text.data );
            return false;
        }

        ctx.tokens.push_back(new_token);
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "%4llu) \"%s\" \n", new_token.index, new_token.view().c_str() );
    }

    NDBL_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Tokenization.\n%s\n", parser_to_string(ctx).c_str() );
    return true;
}

Token parse_token(const Parser_Context& ctx, bdc::String& buffer)
{
    ASSERT(buffer.size > 0);

    // single-line comment
    if ( buffer.size > 2 && buffer[0] == '/' && buffer[1] == '/')
    {
        u32_t cursor = 2;            
        while ( cursor < buffer.size && buffer[cursor] != '\n' )
        {
            cursor += 1;
        }
        // cursor += 1; (we exclude new line)
        
        String word = {
            buffer.data,
            cursor
        };

        buffer = {
            buffer.data + cursor,
            buffer.size - cursor 
        };
        return Token{ Token_Type_comment, word };
    }
    
    //  multi-line comment
    if ( buffer.size > 4 && buffer[0] == '/' && buffer[1] == '*' && buffer[2] != '/') // requires "/*" + 1 char that is not /, minimal comment is "/**/", "/*/" is invalid
    {
        u32_t cursor = 2;            
        while ( true )
        {
            if( buffer.size <= cursor )
                return Token{ Token_Type_NULL, { buffer.data + cursor, buffer.size - cursor } };
            if (buffer[cursor-1] == '*' && buffer[cursor] != '/')
                break;
            cursor += 1;
        }
        
        String word = {
            buffer.data,
            cursor
        };

        buffer = {
            buffer.data + cursor,
            buffer.size - cursor 
        };
        return Token{ Token_Type_multiline_comment, word };
    }

    // single-char
    auto single_char_found = hashmap_find(ctx.langdef->token_type_by_single_char, buffer[0]); // index lookup
    if( single_char_found.ok )
    {
        String word = bdc::string_lsplit( buffer, 1);

        bdc::string_advance(buffer, word.size );

        return Token{ *single_char_found.value, word };
    }

    // operators
    switch ( buffer[0] )
    {
        case '=':
        {
            bdc::String word;

            // Double char operators starting with "=" ("=>" or "==")
            if (buffer.size > 1 && (buffer[1] == '>' || buffer[1] == '='))
            {
                word = bdc::string_lsplit(buffer, 2);
            }
            // "="
            else
            {
                word = bdc::string_lsplit(buffer, 1);
            }

            string_advance(buffer, word.size);
            return Token{ Token_Type_operator, word };
        }

        case '!':
        case '/':
        case '*':
        case '+':
        case '-':
        case '>':
        case '<':
        {
            // "<operator>=" (do not handle: "++", "--")
            if (buffer.size > 1)
            {
                // 3-chars operators:
                // This is just a single special case for equivalence operator ("<=>")
                // we MUST parse this before "<=" of course, since "<=>" includes "<="
                if (buffer.size > 2 && buffer[0] == '<'  && buffer[1] == '=' && buffer[2] == '>'  )
                {
                    bdc::String word = bdc::string_lsplit(buffer, 3);
                    bdc::string_advance(buffer, word.size);
                    return Token{ Token_Type_operator, word };
                }

                // 2-chars operators: >=, <= += -=, etc.
                if (buffer[1] == '=')
                {                    
                    bdc::String word = bdc::string_lsplit(buffer, 2);
                    bdc::string_advance(buffer, word.size);
                    return Token{ Token_Type_operator, word };
                }
            }

            // single char operator
            bdc::String word = bdc::string_lsplit(buffer, 1);
            bdc::string_advance(buffer, 1);
            return Token{ Token_Type_operator, word };
        }
    }

    // number (double)
    //     note: we accept zeros as prefix (ex: "0002.15454", or "01012")
    if ( std::isdigit( buffer[0] ) )
    {
        u32_t cursor = 1;
        Token_Type type = Token_Type_literal_int;

        // integer
        while (cursor != buffer.size && std::isdigit( buffer[cursor] ))
        {
            ++cursor;
        }

        // double
        if(cursor + 1 < buffer.size
        && buffer[cursor] == '.'      // has a decimal separator
            && std::isdigit(buffer[cursor + 1]) // followed by a digit
        )
        {
            u32_t local_cursor_decimal_separator = cursor;
            ++cursor;

            // decimal portion
            while (cursor != buffer.size && std::isdigit(buffer[cursor]))
            {
                ++cursor;
            }
            type = Token_Type_literal_double;
        }
        bdc::String word = bdc::string_lsplit(buffer, cursor);
        bdc::string_advance(buffer, cursor);
        return Token{type, word };
    }

    // double-quoted string
    if ( buffer[0] == '"')
    {
        u32_t cursor = 1;

        while (cursor != buffer.size && (buffer[cursor] != '"' || buffer[cursor - 1] == '\\'))
        {
            ++cursor;
        }

        if( buffer[cursor] != '"' )
        {
            return Token{ Token_Type_NULL };
        }
        
        ++cursor;
        bdc::String word = bdc::string_lsplit(buffer, cursor);
        bdc::string_advance(buffer, cursor);
        return Token{Token_Type_literal_string, word};
    }

    // symbol (identifier or keyword)
    if ( std::isalpha( buffer[0] ) || buffer[0] == '_' )
    {
        // parse symbol
        u32_t cursor = 1;
        while (cursor < buffer.size && (std::isalnum( buffer[cursor]) || buffer[cursor] == '_') )
        {
            ++cursor;
        }
        
        bdc::String word = bdc::string_lsplit(buffer, cursor );
        bdc::string_advance(buffer, cursor);

        // symbol might be a reserved keyword, let's seach in the keyword index...
        String_Hash word_hash = string_hash(word);
        auto keyword_found = hashmap_find(ctx.langdef->token_type_by_keyword, word_hash.hash );
        if ( keyword_found )
        {            
            return Token{ *keyword_found.value, word };
        }

        // ...otherwise, symbol is an identifier
        return Token{ Token_Type_identifier, word};
        
    }
    return Token{ Token_Type_NULL };
}


Token& parser_push(Parser_Context& ctx, Token& token)
{
    token.index = ctx.tokens.size();
    ctx.tokens.push_back(token);
    return ctx.tokens.back();
}

String parser_to_string(const Parser_Context& ctx)
{
    String_Builder sb;
    string_builder_init(sb);
    string_builder_append(sb, NDBL_COLOR_DEFAULT);

    string_builder_append(sb, "Logging token ribbon state:\n");
    string_builder_append(sb, "___________[TOKEN RIBBON]_________\n");

    for (const Token& token : ctx.tokens)
    {
        if ( token.index == 0 )
        {
            string_builder_append(sb, "B"); // begin
        }
        else if ( token.index == ctx.tokens.back().index )
        {
            string_builder_append(sb, "E"); // end
        }
        else
        {
            string_builder_append(sb, "|"); // default
        }

        if ( !ctx.transaction.empty()
              && token.index >= ctx.transaction.top()
              && token.index <= ctx.cursor )
        {
            string_builder_append(sb, "T"); // transaction
        }
        else
        {
            string_builder_append(sb, "."); // no transaction
        }

        string_builder_appendf(sb, "%5zu) \"%s\"", token.index, token.word_view().c_str() );
      
        if ( token.index == ctx.cursor )
        {
            string_builder_append(sb, " [c]"); // current
        }
        
        string_builder_append(sb, "\n");
    }

    return string_builder_build_tstring(sb).c_str();
}

String parser_to_string(Parser_Context& ctx, size_t begin, size_t end)
{
    ASSERT(begin <= end);
    ASSERT(end <= ctx.tokens.size() );

    String_Builder sb;
    for( size_t i = begin; i < end; ++i )
    {
        string_builder_append(sb, ctx.tokens[i].view() );
    }
    return string_builder_build_tstring(sb);
}

Token parser_eat_if(Parser_Context& ctx, Token_Type expectedType)
{
    if (parser_can_eat(ctx) && parser_peek(ctx).type == expectedType )
    {
        return parser_eat(ctx);
    }
    return Token_Type_NULL;
}

const Token& parser_get_eaten(const Parser_Context& ctx)
{
    ASSERT(ctx.cursor > 0);
    return ctx.tokens[ctx.cursor - 1];
}

bool parser_peek(const Parser_Context& ctx, Token_Type t)
{
    return ctx.cursor < ctx.tokens.size() && ctx.tokens[ctx.cursor].type == t;
}

const Token& parser_peek(const Parser_Context& ctx)
{
    return ctx.tokens[ctx.cursor];
}

Token parser_eat(Parser_Context& ctx)
{
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Token_Ribbon", "Eat token (idx %i) %s \n", ctx.cursor, parser_peek(ctx).view().c_str() );
    return ctx.tokens.at(ctx.cursor++);
}

void parser_start_transaction(Parser_Context& ctx)
{
    ctx.transaction.push(ctx.cursor);
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Token_Ribbon", "Start Transaction (idx %i)\n", ctx.cursor);
}

void parser_rollback(Parser_Context& ctx)
{
    ctx.cursor = ctx.transaction.top();
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Token_Ribbon", "Rollback (idx %i)\n", ctx.cursor);
    ctx.transaction.pop();
}

void parser_commit(Parser_Context& ctx)
{
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Token_Ribbon", "Commit (idx %i)\n", ctx.cursor);
    ctx.transaction.pop();
}

bool parser_can_eat(const Parser_Context& ctx, size_t token_count)
{
    ASSERT(token_count > 0);
    return ctx.cursor + token_count <= ctx.tokens.size() ;
}

Node_Slot* parse_function_call(Parser_Context& ctx, Scope* parent_scope)
{
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "parse function call...\n");

    // Check if the minimum token count required is available ( 0: identifier, 1: open parenthesis, 2: close parenthesis)
    if (!parser_can_eat(ctx, 3))
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " 3 tokens min. are required\n");
        return nullptr;
    }

    parser_start_transaction(ctx);

    // Try to parse regular function: function(...)
    bdc::String function_identifier;
    Token token_0 = parser_eat(ctx);
    Token token_1 = parser_eat(ctx);
    if (token_0.type == Token_Type_identifier &&
        token_1.type == Token_Type_parenthesis_open)
    {
        function_identifier = token_0.word_view();
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Regular function pattern detected.\n");
    }
    else // Try to parse operator like (ex: operator==(..,..))
    {
        Token token_2 = parser_eat(ctx);// eat a "supposed open bracket>

        if (token_0.type == Token_Type_keyword_operator && token_1.type == Token_Type_operator && token_2.type == Token_Type_parenthesis_open)
        {
            function_identifier = token_1.word_view();// operator
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Operator function-like pattern detected.\n");
        }
        else
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Not a function.\n");
            parser_rollback(ctx);
            return nullptr;
        }
    }
    bdc::Resizable_Array<Node_Slot*> result_slots;
    array_init(result_slots, 16, &temp_allocator);

    // Declare a new function prototype
    Type_Descriptor function_type;
    type_init<any()>(&function_type);
    function_type.name = function_identifier;

    bool parsingError = false;
    while (!parsingError && parser_can_eat(ctx) &&
        parser_peek(ctx).type != Token_Type_parenthesis_close)
    {
        Node_Slot* expression_out = parse_expression(ctx, parent_scope);
        if ( expression_out )
        {
            array_append(result_slots, expression_out );
            function_type.function_push_arg( expression_out->property->type );
            parser_eat_if(ctx, Token_Type_list_separator);
        }
        else
        {
            parsingError = true;
        }
    }

    // eat "close bracket supposed" token
    if ( !parser_eat_if(ctx, Token_Type_parenthesis_close) )
    {
        NDBL_LOG(Verbosity_Warning, "Parser", NDBL_KO " Expecting parenthesis close\n");
        parser_rollback(ctx);
        return nullptr;
    }


    // Find the prototype in the ctx library
    Node* fct_node = graph_create_function( ctx.out_graph, &function_type, parent_scope );

    for ( int i = 0; i < fct_node->component.invokable.argument_slots.size; i++ )
    {
        // Connects each results to the corresponding input
        graph_connect_or_merge(result_slots[i], fct_node->component.invokable.argument_slots[i] );
    }

    parser_commit(ctx);
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Function call parsed:\n%s\n", parser_to_string(ctx).c_str() );

    return fct_node->value_out();
}

Node* parse_atomic_code_block(Parser_Context& ctx, Scope* parent_scope, Node_Slot* flow_out)
{
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing atomic code block ..\n");
    ASSERT(flow_out);

    // most common case
    Node* block = nullptr;
            if ( (block = parse_scoped_block(ctx, parent_scope, flow_out)) );
    else if ( (block = parse_return(ctx, parent_scope, flow_out)));
    else if ( (block = parse_expression_block(ctx, parent_scope, flow_out)) );
    else if ( (block = parse_if_block(ctx, parent_scope, flow_out)) );
    else if ( (block = parse_for_block(ctx, parent_scope, flow_out)) );
    else if ( (block = parse_while_block(ctx, parent_scope, flow_out)) ) ;
    else      (block = parse_empty_block(ctx, parent_scope, flow_out));

    if ( block )
    {
        if ( Token tok = parser_eat_if(ctx, Token_Type_end_of_instruction) )
        {
            block->suffix = tok;
        }

        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Block found (class \"%s\")\n", block->get_class()->name.c_str() );
        return block;
    }

    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " No block found\n");
    return nullptr;
}

Node* parse_if_block(Parser_Context& ctx, Scope* parent_scope, Node_Slot* flow_out)
{
    parser_start_transaction(ctx);

    Token if_token = parser_eat_if(ctx, Token_Type_keyword_if);
    if ( !if_token )
    {
        return nullptr;
    }

    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing if statement...\n");

    // if
    Node* if_node  = graph_create_cond_struct( ctx.out_graph, parent_scope );
    if_node->component.branching.branch_prefix = parser_get_eaten(ctx);

    graph_connect(flow_out, if_node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );

    if ( !parser_eat_if(ctx, Token_Type_parenthesis_open) )
    {
        graph_find_and_destroy_node(ctx.out_graph, if_node);
        parser_rollback(ctx);
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Open bracket expected\n");
        return nullptr;
    }
    
    // if's condition
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing if block's condition...\n");
    parse_expression_block( ctx, if_node->internal_scope, nullptr, if_node->component.branching.condition_in());

    if ( !parser_eat_if(ctx, Token_Type_parenthesis_close) )
    {
        graph_find_and_destroy_node(ctx.out_graph, if_node);
        parser_rollback(ctx);
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Close bracket expected\n");
        return nullptr;
    }

    // if's block
    Node* if_block = parse_atomic_code_block( ctx,  if_node->internal_scope, if_node->component.branching.branch_out(Branch_TRUE) );
    if( if_block )
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " if block parsed\n");
    }

    // else (optionnal)
    if ( parser_eat_if(ctx, Token_Type_keyword_else) )
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing else statement ...\n");
        if_node->component.branching.branch_suffix = parser_get_eaten(ctx);

        // else's block
        if ( Node* else_block = parse_atomic_code_block( ctx,  if_node->internal_scope, if_node->component.branching.branch_out(Branch_FALSE) ) )
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " else block parsed.\n");
        }
        else
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " else block parsed (without code)\n");
        }
    }                   

    parser_commit(ctx);
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Parse conditional structure:\n%s\n", parser_to_string(ctx).c_str() );
    // TODO: connect true/false branches flow_out to scope flow_leave?"
    return if_node;
}

Node* parse_for_block(Parser_Context& ctx, Scope* parent_scope, Node_Slot* flow_out)
{
    bool        success     = false;
    Node*    for_node    = nullptr;

    parser_start_transaction(ctx);

    if ( Token token_for = parser_eat_if(ctx, Token_Type_keyword_for) )
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing for loop ...\n");

        for_node = graph_create_for_loop( ctx.out_graph, parent_scope );
        for_node->component.branching.branch_prefix = token_for;

        graph_connect( flow_out, for_node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );

        Token open_bracket = parser_eat_if(ctx, Token_Type_parenthesis_open);
        if ( open_bracket)
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing for set_name/condition/iter instructions ...\n");

            // first we parse three instructions, no matter if we find them, we'll continue (we are parsing something abstract)

            // parse init; condition; iteration or nothing
            parse_expression_block(ctx, for_node->internal_scope, nullptr, for_node->component.branching.initialization_slot)
            && parse_expression_block(ctx, for_node->internal_scope, nullptr, for_node->component.branching.condition_in())
            && parse_expression_block(ctx, for_node->internal_scope, nullptr, for_node->component.branching.iteration_slot);

            // parse parenthesis close
            if ( Token parenthesis_close = parser_eat_if(ctx, Token_Type_parenthesis_close) )
            {
                Node* block = parse_atomic_code_block( ctx,  for_node->internal_scope, for_node->component.branching.branch_out(Branch_TRUE) ) ;

                if ( block )
                {
                    success = true;
                    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Scope or single instruction found\n");
                }
                else
                {
                    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Scope or single instruction expected\n");
                }
            }
            else
            {
                NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Close parenthesis was expected.\n");
            }
        }
        else
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Open parenthesis was expected.\n");
        }
    }

    if ( success )
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " For block parsed\n");
        parser_commit(ctx);
        // TODO: Should we connect true/false branches to scope's flow_leave Node_Slot?
        return for_node;
    }

    if ( for_node )
    {
        graph_find_and_destroy_node(ctx.out_graph, for_node);
    }
    parser_rollback(ctx);
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Could not parse for block\n");
    return {};
}

Node* parse_while_block(Parser_Context& ctx, Scope* parent_scope, Node_Slot* flow_out)
{
    bool        success     = false;
    Node*    while_node  = nullptr;
    Node*    block       = nullptr;

    parser_start_transaction(ctx);

    if ( Token token_while = parser_eat_if(ctx, Token_Type_keyword_while) )
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing while ...\n");

        while_node = graph_create_while_loop( ctx.out_graph, parent_scope );
        while_node->component.branching.branch_prefix = token_while;

        graph_connect( flow_out, while_node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );

        if ( Token open_bracket = parser_eat_if(ctx, Token_Type_parenthesis_open) )
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing while condition ... \n");

            // Parse an optional condition
            parse_expression_block( ctx, while_node->internal_scope, nullptr, while_node->component.branching.condition_in());

            if (parser_eat_if(ctx, Token_Type_parenthesis_close) )
            {
                block = parse_atomic_code_block( ctx,  while_node->internal_scope, while_node->component.branching.branch_out(Branch_TRUE) );
                if ( block )
                {
                    success = true;
                }
                else
                {
                    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO "  Scope or single instruction expected\n");
                }
            }
            else
            {
                NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO "  Parenthesis close expected\n");
            }
        }
        else
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO "  Parenthesis close expected\n");
        }
    }

    if ( success )
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing while:\n%s\n", parser_to_string(ctx).c_str() );
        parser_commit(ctx);
        // TODO: Should we connect true/false branches to scope's flow_leave SLot?
        return while_node;
    }

    parser_rollback(ctx);
    graph_find_and_destroy_node(ctx.out_graph, while_node);
    graph_find_and_destroy_node(ctx.out_graph, block);

    return {};
}

Node* parse_return(Parser_Context& ctx, Scope* parent_scope, Node_Slot* flow_out)
{
    if (!parser_can_eat(ctx, 2))
    {
        return nullptr;
    }

    parser_start_transaction(ctx);

    if ( Token return_token = parser_eat_if(ctx, Token_Type_keyword_return) )
    {
        // Parse the expression at the right side of the return
        if ( Node_Slot* expression_out = parse_expression(ctx, parent_scope) )
        {
            const Type_Descriptor* type = expression_out->property->type;
            Node* return_node = graph_create_return( ctx.out_graph, type, parent_scope );
            return_node->value->token = return_token;

            // TODO: assign prefix and suffix to return Node

            // Connect the expression to the return Node
            graph_connect(expression_out, return_node->value_in());
            // and to the flow
            graph_connect(flow_out, return_node->flow_in());

            parser_commit(ctx);
            return return_node;
        }
    }

    parser_rollback(ctx);
    return nullptr;
}

Node_Slot* parse_variable_declaration(Parser_Context& ctx, Scope* parent_scope)
{
    if (!parser_can_eat(ctx, 2))
    {
        return nullptr;
    }

    parser_start_transaction(ctx);

    bool  success          = false;
    Token type_token       = parser_eat(ctx);
    Token identifier_token = parser_eat(ctx);

    if (type_token.is_keyword_type() && identifier_token.type == Token_Type_identifier)
    {
        const Type_Descriptor* type = langdef_get_type_descriptor_from_token_type(*ctx.langdef, type_token.type);
        Node* variable_node = graph_create_variable( ctx.out_graph, type, identifier_token.word_view(), parent_scope );

        SET_FLAGS(variable_node->component.variable.flags, VariableFlag_DECLARED);
        variable_node->component.variable.type_token = type_token;
        node_set_identifier_token(variable_node, identifier_token );

        // declaration with assignment ?
        Token operator_token = parser_eat_if(ctx, Token_Type_operator);
        if (operator_token && operator_token.word_view() == "=")
        {
            // an expression is expected
            if ( Node_Slot* expression_out = parse_expression(ctx, parent_scope) )
            {
                // expression's out ----> variable's in
                graph_connect_to_variable(expression_out, variable_node );

                variable_node->component.variable.operator_token = operator_token;
                success = true;
            }
            else
            {
                NDBL_DEBUG_LOG(
                    Verbosity_Diagnostic, "Parser", 
                    NDBL_KO "  Initialization expression expected for %s\n", identifier_token.word_view().c_str());
            }
        }
            // Declaration without assignment
        else
        {
            success = true;
        }

        if ( success )
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Variable declaration: %s %s\n",
                        variable_node->value->type->name.c_str(),
                        identifier_token.word_view().c_str());
            parser_commit(ctx);
            return variable_node->value_out();
        }

        NDBL_DEBUG_LOG(
            Verbosity_Diagnostic, "Parser", 
            NDBL_KO "  Initialization expression expected for %s\n", identifier_token.word_view().c_str());
        graph_find_and_destroy_node(ctx.out_graph, variable_node);
    }

    parser_rollback(ctx);
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------------
//
//                                  SERIALIZATION
//
//---------------------------------------------------------------------------------------------------------------------------

void serializer_init(Serializer_Context& ctx, Graph* in_graph )
{
    ctx.langdef  = &langdef();
    ctx.in_graph = in_graph;
    string_builder_init(ctx.out_sb);

    serializer_reset(ctx, in_graph);
}

void serializer_deinit(Serializer_Context& ctx )
{
    string_builder_release(ctx.out_sb);
}

void serializer_reset(Serializer_Context& ctx, Graph* in_graph )
{
    ctx.in_graph = in_graph;
    string_builder_reset(ctx.out_sb);
}

const Node_Slot* serialize_invokable(Serializer_Context& ctx, const Node* _node)
{
    if (_node->type == Node_Type_OPERATOR )
    {
        Array<Node_Slot*> args = array_view( _node->component.invokable.argument_slots );
        int precedence = langdef_get_precedence(*ctx.langdef, &_node->component.invokable.type);

        switch ( _node->component.invokable.type.function.args.size )
        {
            case 2:
            {
                // Left part of the expression
                {
                    const Type_Descriptor* l_func_type = node_get_connected_function_type(_node, LEFT_VALUE_PROPERTY);
                    bool needs_braces = l_func_type && langdef_get_precedence(*ctx.langdef, l_func_type) < precedence;
                    Serialization_Flags flags = Serialization_Flag_RECURSE
                                        | needs_braces * Serialization_Flag_WRAP_WITH_BRACES ;
                    serialize_input( ctx, args[0], flags );
                }

                // Operator
                VERIFY( _node->component.invokable.identifier_token, "identifier token should have been assigned in parse_function_call");
                string_builder_append( ctx.out_sb, serialize_token( ctx, _node->component.invokable.identifier_token ));

                // Right part of the expression
                {
                    const Type_Descriptor* r_func_type = node_get_connected_function_type(_node, RIGHT_VALUE_PROPERTY);
                    bool needs_braces = r_func_type && langdef_get_precedence(*ctx.langdef, r_func_type) < precedence;
                    Serialization_Flags flags = Serialization_Flag_RECURSE
                                        | needs_braces * Serialization_Flag_WRAP_WITH_BRACES ;
                    serialize_input( ctx, args[1], flags );
                }
                break;
            }

            case 1:
            {
                // operator ( ... innerOperator ... )   ex:   -(a+b)

                ASSERT( _node->component.invokable.identifier_token );
                string_builder_append( ctx.out_sb, serialize_token( ctx, _node->component.invokable.identifier_token) );

                bool needs_braces    = node_get_connected_function_type(_node, LEFT_VALUE_PROPERTY) != nullptr;
                Serialization_Flags flags = Serialization_Flag_RECURSE
                                    | needs_braces * Serialization_Flag_WRAP_WITH_BRACES;
                serialize_input( ctx, args[0], flags );
                break;
            }
        }
    }
    else
    {
        serialize_function_call(ctx, &_node->component.invokable.type, array_view(_node->component.invokable.argument_slots) );
    }

    return _node->value_out();
}

void serialize_function_call(Serializer_Context& ctx, const Type_Descriptor *function_type, const bdc::Array<Node_Slot*>& inputs)
{
    string_builder_append( ctx.out_sb, function_type->name );
    
    string_builder_append( ctx.out_sb, serialize_token_type(ctx, Token_Type_parenthesis_open));

    for (const Node_Slot* input_slot : inputs)
    {
        ASSERT( HAS_FLAGS(input_slot->flags, Node_Slot::Flag_INPUT) );
        if ( input_slot != inputs[0])
        {
            string_builder_append( ctx.out_sb, serialize_token_type(ctx, Token_Type_list_separator));
        }
        serialize_input( ctx, input_slot, Serialization_Flag_RECURSE );
    }

    string_builder_append( ctx.out_sb, serialize_token_type(ctx, Token_Type_parenthesis_close) );
}

void serialize_function_type(Serializer_Context& ctx, const Type_Descriptor *function_type)
{
    string_builder_append( ctx.out_sb, serialize_type(ctx, function_type->function.return_type));
    string_builder_append( ctx.out_sb, " ");
    string_builder_append( ctx.out_sb, function_type->name );
    string_builder_append( ctx.out_sb, serialize_token_type(ctx, Token_Type_parenthesis_open) );

    for (auto it = function_type->function.args.begin(); it != function_type->function.args.end(); it++)
    {
        if (it != function_type->function.args.begin())
        {
            string_builder_append( ctx.out_sb, serialize_token_type(ctx, Token_Type_list_separator));
            string_builder_append( ctx.out_sb, " ");
        }
        string_builder_append( ctx.out_sb, serialize_type(ctx, (*it).type) );
    }

    string_builder_append( ctx.out_sb, serialize_token_type(ctx, Token_Type_parenthesis_close));
}

void serialize_variable_ref(Serializer_Context& ctx, const Node* _node)
{
    ASSERT(_node->type == Node_Type_VARIABLE_REF);
    String token_str = serialize_token( ctx, node_get_identifier_token(_node) );
    string_builder_append( ctx.out_sb, token_str);
}

void serialize_variable(Serializer_Context& ctx, const Node *_node)
{
    ASSERT(_node->type == Node_Type_VARIABLE);

    // 1. Serialize variable's type

    // If parsed
    if ( _node->component.variable.type_token )
    {
        string_builder_append(ctx.out_sb, serialize_token( ctx, _node->component.variable.type_token) );
    }
    else // If created in the graph by the user
    {
        string_builder_append(ctx.out_sb, serialize_type(ctx, _node->value->type) );
        string_builder_append(ctx.out_sb, " ");
    }

    // 2. Serialize variable identifier
    string_builder_append(ctx.out_sb, serialize_token( ctx, node_get_identifier_token(_node) ));

    // 3. Initialisation
    //    When a VariableNode has its input connected, we serialize it as its initialisation expression

    const Node_Slot* slot = _node->value_in();
    if ( slot->adjacent.size != 0 )
    {
        if ( _node->component.variable.operator_token )
            string_builder_append(ctx.out_sb, _node->component.variable.operator_token.view());
        else
            string_builder_append(ctx.out_sb, " = ");

        serialize_input( ctx, slot, Serialization_Flag_RECURSE );
    }
}

void serialize_return(Serializer_Context& ctx, const Node* node)
{
    ASSERT(node->type == Node_Type_RETURN);

    if( node->value->token )
    {
        string_builder_append(ctx.out_sb, serialize_token( ctx, node->value->token ));
    }
    else
    {
        string_builder_append(ctx.out_sb, *hashmap_find(ctx.langdef->keyword_by_token_type, Token_Type_keyword_return).value );
        string_builder_append(ctx.out_sb, " ");
    }

    if ( const Node_Slot* input_slot = node->value_in() )
    {
        serialize_input( ctx, input_slot, Serialization_Flag_RECURSE );
    }
}

void serialize_input(Serializer_Context& ctx, const Node_Slot* slot, Serialization_Flags _flags )
{
    ASSERT( HAS_FLAGS(slot->flags, Node_Slot::Flag_INPUT ) );

    const Node_Slot*     adjacent_slot     = slot->first_adjacent();
    const Node_Property* adjacent_property = adjacent_slot != nullptr ? adjacent_slot->property
                                                                        : nullptr;
    // Append open brace?
    if ( _flags & Serialization_Flag_WRAP_WITH_BRACES )
        string_builder_append(ctx.out_sb, serialize_token_type(ctx,  Token_Type_parenthesis_open));

    if ( adjacent_property == nullptr )
    {
        // Simply serialize this property
        serialize_property(ctx, slot->property);
    }
    else
    {
        VERIFY( _flags & Serialization_Flag_RECURSE, "Why would you call serialize_input without RECURSE flag?");
        // Append token prefix?
        if (adjacent_property->token)
            string_builder_append(ctx.out_sb, adjacent_property->token.prefix_view());

        // Serialize adjacent slot
        serialize_node_value_out(ctx, adjacent_slot, Serialization_Flag_RECURSE);

        // Append token suffix?
        if (adjacent_property->token )
                string_builder_append(ctx.out_sb, adjacent_property->token.suffix_view());
    }

    // Append close brace?
    if ( _flags & Serialization_Flag_WRAP_WITH_BRACES )
        string_builder_append( ctx.out_sb, serialize_token_type(ctx, Token_Type_parenthesis_close));
}

void serialize_node_value_out(Serializer_Context& ctx, const Node_Slot* slot, Serialization_Flags _flags)
{
    // If output is node's output value, we serialize the node
    if( slot == slot->node->value_out() )
    {
        return serialize_node(ctx, slot->node, _flags);
        return;
    }

    // Otherwise, it might be a variable reference, so we serialize the identifier only
    ASSERT(slot->node->type == Node_Type_VARIABLE ); // Can't be another type
    VERIFY( slot == slot->node->component.variable.ref_out, "Cannot serialize an other slot from a VariableNode");
    string_builder_append(  ctx.out_sb, node_get_identifier(slot->node) );
}

void serialize_node(Serializer_Context& ctx, const Node* node, Serialization_Flags _flags )
{
    if ( node == nullptr )
        return;

    ASSERT( _flags == Serialization_Flag_RECURSE ); // The only flag configuration handled for now

    switch ( node->type )
    {
        case Node_Type_RETURN:            serialize_return(ctx, node);                   break;
        case Node_Type_IF_ELSE:           serialize_if_else(ctx, node);                  break;
        case Node_Type_FOR_LOOP:          serialize_for_loop(ctx, node);                 break;
        case Node_Type_WHILE_LOOP:        serialize_while_loop(ctx, node);               break;
        case Node_Type_LITERAL:           serialize_literal(ctx, node);                  break;
        case Node_Type_VARIABLE:          serialize_variable(ctx, node);                 break;
        case Node_Type_VARIABLE_REF:      serialize_variable_ref(ctx, node);             break;
        case Node_Type_FUNCTION:          [[fallthrough]];        
        case Node_Type_OPERATOR:          serialize_invokable(ctx, node);                break;
        case Node_Type_EMPTY_INSTRUCTION: serialize_empty_instruction(ctx, node);        break;
        case Node_Type_ROOT:              [[fallthrough]];
        case Node_Type_SCOPE:             serialize_scope(ctx, node->internal_scope );   break;
        default:                          VERIFY(false, "Unhandled NodeType, can't serialize");
    }

    String token_str = serialize_token( ctx, node->suffix );
    string_builder_append( ctx.out_sb, token_str);
}

void serialize_scope(Serializer_Context& ctx, const Scope* scope)
{
    string_builder_append( ctx.out_sb, serialize_token( ctx, scope->token_begin) );
    
    for(Node* node : scope_get_backbone(scope) )
    {
        serialize_node( ctx, node, Serialization_Flag_RECURSE);
    }
    
    string_builder_append( ctx.out_sb, serialize_token( ctx, scope->token_end) );
}

bdc::String serialize_bool(const Serializer_Context& ctx, bool b)
{
    return b ? "true" : "false";
}

bdc::String serialize_int(const Serializer_Context& ctx, int i)
{
    return string_printf( "%i", i );
}

bdc::String serialize_double(const Serializer_Context& ctx, double d)
{
    return string_printf( "%d", d );
}

bdc::String serialize_token(const Serializer_Context& ctx, const Token& token)
{
    return token.view();
}

bdc::String serialize_token_type(const Serializer_Context& ctx, Token_Type token_type)
{
    switch (token_type)
    {
        case Token_Type_end_of_line:     return "\n"; // TODO: handle all platforms
        case Token_Type_operator:        return "operator";
        case Token_Type_identifier:      return "identifier";
        case Token_Type_literal_string:  return "\"\"";
        case Token_Type_literal_double:  return "0.0";
        case Token_Type_literal_int:     return "0";
        case Token_Type_literal_bool:    return "false";
        case Token_Type_literal_any:     return "0";
        case Token_Type_NULL:          [[fallthrough]];
        case Token_Type_literal_unknown: return "";
        default:
        {
            if (auto found = hashmap_find(ctx.langdef->keyword_by_token_type, token_type))
            {
                return *found.value;
            }
            if (auto found = hashmap_find(ctx.langdef->single_char_by_keyword, token_type))
            {
                return String{*found.value};
            }
            return "<?>";
        }
    }
}

void serialize_graph(Serializer_Context& ctx)
{
    VERIFY(ctx.in_graph != nullptr, "Did you call serialize_begin()");
    const Node* root_node = graph_root(ctx.in_graph);
    if ( root_node == nullptr )
    {
        NDBL_LOG(Verbosity_Error, "Serializer", "a root primary_child is expected to serialize the graph\n");
        return;
    }
    serialize_node(ctx, root_node, Serialization_Flag_RECURSE);
}

void serialize_for_loop(Serializer_Context& ctx, const Node* _for_loop)
{
    ASSERT( _for_loop->type == Node_Type_FOR_LOOP );

    string_builder_append( ctx.out_sb, serialize_token( ctx, _for_loop->component.branching.branch_prefix) );
    string_builder_append( ctx.out_sb, serialize_token_type(ctx, Token_Type_parenthesis_open) );
    {
        const Node_Slot* init_slot = node_find_slot_by_property_name(_for_loop, INITIALIZATION_PROPERTY, Node_Slot::Flag_INPUT );
        const Node_Slot* cond_slot = node_find_slot_by_property_name(_for_loop, CONDITION_PROPERTY, Node_Slot::Flag_INPUT );
        const Node_Slot* iter_slot = node_find_slot_by_property_name(_for_loop, ITERATION_PROPERTY, Node_Slot::Flag_INPUT );
        serialize_input( ctx, init_slot, Serialization_Flag_RECURSE );
        serialize_input( ctx, cond_slot, Serialization_Flag_RECURSE );
        serialize_input( ctx, iter_slot, Serialization_Flag_RECURSE );
    }
    string_builder_append( ctx.out_sb, serialize_token_type(ctx, Token_Type_parenthesis_close) );
    serialize_node( ctx, _for_loop->component.branching.branch_out(Branch_TRUE)->first_adjacent_node(), Serialization_Flag_RECURSE );
}

void serialize_while_loop(Serializer_Context& ctx, const Node* _while_loop_node)
{
    ASSERT( _while_loop_node->type == Node_Type_WHILE_LOOP );

    // while
    String while_str = serialize_token( ctx, _while_loop_node->component.branching.branch_prefix);
    string_builder_append(ctx.out_sb, while_str);

    // condition
    Serialization_Flags flags = Serialization_Flag_RECURSE
                        | Serialization_Flag_WRAP_WITH_BRACES;
    serialize_input( ctx, _while_loop_node->component.branching.condition_in(), flags );

    if ( const Node* _node = _while_loop_node->component.branching.branch_out(Branch_TRUE)->first_adjacent_node() )
    {
        serialize_node( ctx, _node, Serialization_Flag_RECURSE);
    }
}

void serialize_if_else(Serializer_Context& ctx, const Node* if_node )
{
    ASSERT( if_node->type == Node_Type_IF_ELSE );

    // if
    String if_str = serialize_token( ctx, if_node->component.branching.branch_prefix );
    string_builder_append(ctx.out_sb,  if_str );

    // condition
    Serialization_Flags flags = Serialization_Flag_RECURSE
                        | Serialization_Flag_WRAP_WITH_BRACES;
    serialize_input(ctx, if_node->component.branching.condition_in(), flags );

    // when condition is true
    serialize_node(ctx, if_node->component.branching.branch_out(Branch_TRUE)->first_adjacent_node(), Serialization_Flag_RECURSE );

    // when condition is false
    string_builder_append(ctx.out_sb, serialize_token( ctx, if_node->component.branching.branch_suffix) );
    serialize_node(ctx, if_node->component.branching.branch_out(Branch_FALSE)->first_adjacent_node(), Serialization_Flag_RECURSE );
}

void serialize_property(Serializer_Context& ctx, const Node_Property* property)
{
    const String property_str = serialize_token( ctx, property->token);
    string_builder_append( ctx.out_sb, property_str );
}

bdc::String serialize_type(const Serializer_Context& ctx, const Type_Descriptor* type)
{
    if (auto found = hashmap_find(ctx.langdef->keyword_by_type_id, type->id.hash_code() ))
    {
        return *found.value;
    }
    return "";
}

void serialize_literal(Serializer_Context& ctx, const Node* node)
{
    ASSERT( node->type == Node_Type_LITERAL );
    serialize_property( ctx, node->value );
}

void serialize_empty_instruction(Serializer_Context& ctx, const Node* node)
{
    ASSERT( node->type == Node_Type_EMPTY_INSTRUCTION );
    string_builder_append( ctx.out_sb, serialize_token( ctx, node->value->token ) );
}

String serializer_build_tstring(Serializer_Context& ctx)
{
    String result = string_builder_build_tstring(ctx.out_sb);
    return result;
}

String serializer_build_string(Serializer_Context& ctx)
{
    String result = string_builder_build_string(ctx.out_sb);
    return result;
}

} // namespace ndbl