//---------------------------------------------------------------------------------------------------------------------------
// Parser_Context.cpp
// This file is structured in 3 parts, use Ctrl + F to search:
//  [SECTION] A. Declaration (types, keywords, etc.)
//  [SECTION] B. Parser_Context
//  [SECTION] C. Serializer
//---------------------------------------------------------------------------------------------------------------------------

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

void            _reset_graph(Parser_Context&, Graph*);
bdc::String     parser_ribbon_to_string(const Parser_Context&);
Graph*          parser_graph(const Parser_Context&);
bdc::String     _rsplit_buffer(const Parser_Context&, size_t offset);
bool            _accepts_suffix(const Parser_Context&, Token_Type);
bool            _is_syntax_valid(const Parser_Context&); // Check if the syntax of the token ribbon is correct. (ex: ["12", "-"] is incorrect)

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

    // A.1. Define the parser
    //-------------------------
    array_init(langdef->chars);

    array_append(langdef->chars, {
        { '(',  Token_Type_parenthesis_open},
        { ')',  Token_Type_parenthesis_close},
        { '{',  Token_Type_scope_begin},
        { '}',  Token_Type_scope_end},
        { '\n', Token_Type_ignore},
        { '\t', Token_Type_ignore},
        { ' ',  Token_Type_ignore},
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
    VERIFY(g_langdef, "No parser found, did you call init_language?");
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

bool _accepts_suffix(const Parser_Context& parser, Token_Type type)
{
    return type != Token_Type_identifier          // identifiers must stay clean because they are reused
            && type != Token_Type_parenthesis_open    // ")" are lost when creating AST
            && type != Token_Type_parenthesis_close;  // "(" are lost when creating AST
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
//                                  PARSER (BASICS)
//
//---------------------------------------------------------------------------------------------------------------------------

void parser_init(Parser_Context& parser)
{
    parser.langdef = &langdef();    // langdef if unique for now...
    string_builder_init(parser.sb); // for serialization output
}

void parser_deinit(Parser_Context& parser)
{
    string_builder_release(parser.sb);
}

bdc::String parser_ribbon_to_string(const Parser_Context& parser)
{
    return parser.ribbon.to_string();
};

Graph* parser_graph(const Parser_Context& parser)
{
    ASSERT(parser.graph);
    return parser.graph;
}

void parser_reset(Parser_Context& parser, Graph* graph, String buffer)
{
    parser.buffer = buffer;
    parser.ribbon.reset( buffer );
    parser.graph = graph;
    graph_reset( parser.graph );
}

void reset_graph(Parser_Context& parser, Graph* new_graph)
{
    parser.graph = new_graph; // memory not owned
}

//---------------------------------------------------------------------------------------------------------------------------
//
//                                  PARSING
//
//---------------------------------------------------------------------------------------------------------------------------

bool parse(Parser_Context& parser, Graph* graph_out, bdc::String code)
{
    parser_reset(parser, graph_out, code);

    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing ...\n%s\n", code.c_str() );

    if ( !tokenize(parser, code) )
    {
        return false;
    }

    if ( !_is_syntax_valid(parser) )
    {
        return false;
    }

    Scope* scope = parse_program(parser);

    if ( parser.ribbon.can_eat() )
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " End of token ribbon expected\n");
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "%s", Format::title("Token_Ribbon").c_str());
        for (const Token& each_token : parser.ribbon )
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "token idx %i: %s\n", each_token.index, each_token.json().c_str());
        }
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "%s", Format::title("Token_Ribbon end").c_str());
        auto curr_token = parser.ribbon.peek();
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Failed to parse from token %llu/%llu and above.\n", curr_token.index, parser.ribbon.size());
        NDBL_LOG(Verbosity_Error, "Parser", "Unable to parse all the tokens\n");
        return false;
    }
    return true;
}

bool parse_bool_or(const Parser_Context& parser, bdc::String& buffer, bool default_value)
{
    Token token = parse_token( parser, buffer);
    if (token.type == Token_Type_literal_bool )
        return token.word_view() == "true";
    return default_value;
}

double parse_double_or(const Parser_Context& parser, bdc::String& buffer, double default_value)
{
    Token token  = parse_token( parser, buffer);

    if (token.type == Token_Type_literal_double )
    {
        return std::stod( token.word_view().c_str() );
    }

    return default_value;
}

int parse_int_or(const Parser_Context& parser, bdc::String& buffer, int default_value)
{
    Token token  = parse_token( parser, buffer);

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

Node_Slot* parse_token(const Parser_Context& parser, Scope* parent_scope, const Token& _token)
{
    if (_token.type == Token_Type_identifier)
    {
        bdc::String identifier = _token.word_view();
        if( Node* existing_node = scope_find_variable(parent_scope, identifier) )
        {
            return existing_node->component.variable.ref_out;
        }

        if ( !parser.strict_mode )
        {
            // Insert a VariableNodeRef with "any" type
            NDBL_LOG(Verbosity_Warning,  "Parser", "%s is not declared (strict mode), abstract graph can be generated but compilation will fail.\n",
                        _token.word_view().c_str() );
            Node* ref = graph_create_variable_ref( parser.graph, parent_scope );
            ref->value->token = _token;
            return ref->value_out();
        }

        NDBL_LOG(Verbosity_Error,  "Parser", "%s is not declared (strict mode) \n", _token.word_view().c_str() );
        return nullptr;
    }

    Node* literal = nullptr;

    switch (_token.type)
    {
        case Token_Type_literal_bool:   literal = graph_create_literal<bool>(parser.graph, parent_scope );        break;
        case Token_Type_literal_int:    literal = graph_create_literal<i32_t>( parser.graph, parent_scope );       break;
        case Token_Type_literal_double: literal = graph_create_literal<double>( parser.graph, parent_scope );      break;
        case Token_Type_literal_string: literal = graph_create_literal<bdc::String>( parser.graph, parent_scope ); break;
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

Node_Slot* parse_binary_operation_expression(Parser_Context& parser, Scope* parent_scope, u8_t _precedence, Node_Slot* _left)
{
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing binary expression ...\n");
    ASSERT(_left != nullptr);

    if (!parser.ribbon.can_eat(2))
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Not enough tokens\n");
        return nullptr;
    }

    parser.ribbon.start_transaction();
    const Token operator_token = parser.ribbon.eat();
    const Token operand_token  = parser.ribbon.peek();

    // Structure check
    const bool isValid = operator_token.type == Token_Type_operator &&
                        operand_token.type != Token_Type_operator;

    if (!isValid)
    {
        parser.ribbon.rollback();
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Unexpected tokens\n");
        return nullptr;
    }

    const Operator *ope = langdef_find_operator(*parser.langdef, Operator{ operator_token.word_view(), Operator_Type::Binary} );
    if (ope == nullptr)
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Operator %s not found\n", operator_token.word_view().c_str());
        parser.ribbon.rollback();
        return nullptr;
    }

    // Precedence check
    if (ope->precedence <= _precedence && _precedence > 0)
    {// always update the first operation if they have the same precedence or less.
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Has lower precedence\n");
        parser.ribbon.rollback();
        return nullptr;
    }

    // Parse right expression
    if ( Node_Slot* right = parse_expression(parser, parent_scope, ope->precedence) )
    {
        // Create a function signature according to ltype, rtype and operator word
        Type_Descriptor type;
        type_init<any(any, any)>(&type);
        type.name = ope->identifier;
        type.function.args[0].type = _left->property->type;
        type.function.args[1].type = right->property->type;

        Node* binary_op_node = graph_create_operator( parser.graph, &type, _left->node->scope );

        Node::Invokable_Component& binary_op = binary_op_node->component.invokable;

        binary_op.identifier_token = operator_token;
        binary_op.lvalue_in()->property->token.type = _left->property->token.type;
        binary_op.rvalue_in()->property->token.type = right->property->token.type;

        graph_connect_or_merge(_left, binary_op.lvalue_in());
        graph_connect_or_merge(right, binary_op.rvalue_in() );

        parser.ribbon.commit();
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Binary expression parsed:\n%s\n", parser.ribbon.to_string().c_str());
        return binary_op_node->value_out();
    }

    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Right expression is null\n");
    parser.ribbon.rollback();
    return nullptr;
}

Node_Slot* parse_unary_operation_expression(Parser_Context& parser, Scope* parent_scope, u8_t _precedence)
{
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "parseUnaryOperationExpression...\n");

    if (!parser.ribbon.can_eat(2))
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Not enough tokens\n");
        return nullptr;
    }

    parser.ribbon.start_transaction();
    Token operator_token = parser.ribbon.eat();

    // Check if we get an operator first
    if (operator_token.type != Token_Type_operator)
    {
        parser.ribbon.rollback();
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Expecting an operator token first\n");
        return nullptr;
    }

    // Parse expression after the operator
    Node_Slot* out_atomic = parse_atomic_expression( parser, parent_scope );

    if ( !out_atomic )
    {
        out_atomic = parse_parenthesis_expression( parser, parent_scope );
    }

    if ( !out_atomic )
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Right expression is null\n");
        parser.ribbon.rollback();
        return nullptr;
    }

    // Create a function signature
    Type_Descriptor type;
    type_init<any(any)>(&type);
    type.name = operator_token.word_view();
    type.function.args[0].type = out_atomic->property->type;

    Node* node = graph_create_operator(parser.graph, &type, parent_scope );
    node->component.invokable.identifier_token = operator_token;
    node->component.invokable.lvalue_in()->property->token.type = out_atomic->property->token.type;

    graph_connect_or_merge(out_atomic, node->component.invokable.lvalue_in() );

    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Unary expression parsed:\n%s\n", parser.ribbon.to_string().c_str());
    parser.ribbon.commit();

    return node->value_out();
}

Node_Slot* parse_atomic_expression(Parser_Context& parser, Scope* parent_scope)
{
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing atomic expression ... \n");

    if (!parser.ribbon.can_eat())
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Not enough tokens\n");
        return nullptr;
    }

    parser.ribbon.start_transaction();
    Token token = parser.ribbon.eat();

    if (token.type == Token_Type_operator)
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Cannot start with an operator token\n");
        parser.ribbon.rollback();
        return nullptr;
    }

    if ( Node_Slot* result = parse_token( parser, parent_scope, token) )
    {
        parser.ribbon.commit();
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Atomic expression parsed:\n%s\n", parser.ribbon.to_string().c_str());
        return result;
    }

    parser.ribbon.rollback();
    NDBL_DEBUG_LOG(Verbosity_Diagnostic,  "Parser", NDBL_KO " Unable to parse token (%llu)\n", token.index );

    return nullptr;
}

Node_Slot* parse_parenthesis_expression(Parser_Context& parser, Scope* parent_scope)
{
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "parse parenthesis expr...\n");

    if (!parser.ribbon.can_eat())
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " No enough tokens.\n");
        return nullptr;
    }

    parser.ribbon.start_transaction();
    Token currentToken = parser.ribbon.eat();
    if (currentToken.type != Token_Type_parenthesis_open)
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Open bracket not found.\n");
        parser.ribbon.rollback();
        return nullptr;
    }

    Node_Slot* result = parse_expression(parser, parent_scope);
    if ( result )
    {
        Token token = parser.ribbon.eat();
        if (token.type != Token_Type_parenthesis_close)
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "%s \n", parser.ribbon.to_string().c_str());
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Parenthesis close expected\n",
                        token.word_view().c_str());
            parser.ribbon.rollback();
        }
        else
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Parenthesis expression parsed:\n%s\n", parser.ribbon.to_string().c_str());
            parser.ribbon.commit();
        }
    }
    else
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " No expression after open parenthesis.\n");
        parser.ribbon.rollback();
    }
    return result;
}

Node* parse_expression_block(Parser_Context& parser, Scope* parent_scope, Node_Slot* flow_out, Node_Slot* value_in)
{
    parser.ribbon.start_transaction();

    // Parse an expression
    Node_Slot* value_out = parse_expression(parser, parent_scope);

    // When expression value_out is a variable that is already part of the code flow,
    // we must create a variable reference
    if ( value_out && value_out->node->type == Node_Type_VARIABLE )
    {
        Node* variable = value_out->node;

        if ( node_is_connected_to_codeflow(variable) ) // in such case, we have to reference the variable, since a given variable can't be twice (be declared twice) in the codeflow
        {
            // create a new variable reference
            Node* ref_node = graph_create_variable_ref( parser.graph, parent_scope );
            node_variable_ref_set_variable( ref_node, variable );
            // substitute value_out by variable reference's value_out
            value_out = ref_node->value_out();
        }
    }

    if ( !parser.ribbon.can_eat() )
    {
        // we're passing here if there is no more token, which means we reached the end of file.
        // we allow an expression to end like that.
    }
    else
    {
        // However, in case there are still unparsed tokens, we expect certain type of token, otherwise we reset the result
        switch( parser.ribbon.peek().type )
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
        if (parser.ribbon.peek(Token_Type_end_of_instruction))
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Empty expression found\n");

            Node* empty_instr = graph_create_empty_instruction( parser.graph, parent_scope );
            value_out = empty_instr->value_out();
        }
    }

    // Ensure value_out is defined or rollback transaction
    if ( !value_out )
    {
        parser.ribbon.rollback();
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " parse instruction\n");
        return nullptr;
    }

    // Connects value_out to the provided input
    if ( value_in )
    {
        graph_connect( value_out, value_in, Graph_Flag_ALLOW_SIDE_EFFECTS);
    }

    // Add an end_of_instruction token as suffix when needed
    if (Token tok = parser.ribbon.eat_if(Token_Type_end_of_instruction))
    {
        value_out->node->suffix = tok;
    }

    // Connects expression flow_in with the provided flow_out
    if ( flow_out != nullptr )
    {
        graph_connect( flow_out, value_out->node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );
    }

    // Validate transaction
    parser.ribbon.commit();
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " parse instruction:\n%s\n", parser.ribbon.to_string().c_str());

    return value_out->node;
}

Scope* parse_program(Parser_Context& parser)
{
    VERIFY(parser.graph != nullptr, "A Graph is expected");

    parser.ribbon.start_transaction();

    Scope* scope = graph_root_scope(parser.graph);

    // Parse main code block
    Node* block_last_node = parse_code_block( parser, scope, scope->node->flow_enter() );

    // To preserve any ignored characters stored in the global token
    // we put the prefix and suffix in resp. token_begin and end.
    Token& tok = parser.ribbon.global_token;

    if (tok.prefix_size) scope->token_begin.prefix_push_front( tok.prefix_view() );
    if (tok.suffix_size) scope->token_end.suffix_push_back( tok.suffix_view() );

    if ( parser.ribbon.can_eat( ) )
    {
        parser.ribbon.rollback();
        graph_reset(parser.graph);
        parser.graph->signal_is_complete.emit();
        NDBL_LOG(Verbosity_Warning, "Parser", "Some token remains after getting an empty code block\n");
        NDBL_LOG(Verbosity_Message, "Parser", "Parse program [OK]\n");
        return scope;
    }
    else if ( block_last_node == nullptr )
    {
        NDBL_LOG(Verbosity_Warning, "Parser", "Program main block is empty\n");
    }

    parser.ribbon.commit();
    parser.graph->signal_is_complete.emit();

    NDBL_LOG(Verbosity_Message, "Parser", "Parse program [OK]\n");

    return scope;
}

Node* parse_empty_block(Parser_Context& parser, Scope* parent_scope, Node_Slot* flow_out)
{
    if ( parser.ribbon.peek(Token_Type_end_of_instruction) )
    {
        Node* node = graph_create_empty_instruction( parser.graph, parent_scope );
        graph_connect( flow_out, node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS);
        return node;
    }
    return nullptr;
}

Node* parse_scoped_block(Parser_Context& parser, Scope* parent_scope, Node_Slot* flow_out)
{
    ASSERT(parent_scope);
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing scoped block ...\n");

    Token token_begin = parser.ribbon.eat_if(Token_Type_scope_begin);
    if ( !token_begin )
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Expecting root_scope begin token\n");
        return nullptr;
    }

    parser.ribbon.start_transaction();

    Node* node = graph_create_scope(parser.graph, parent_scope);

    if ( flow_out != nullptr )
        graph_connect( flow_out, node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );


    parse_code_block(parser, node->internal_scope, node->flow_enter()); // no return check, allows empty scope
    Token token_end = parser.ribbon.eat_if(Token_Type_scope_end);

    if ( token_end )
    {
        node->internal_scope->token_begin = token_begin;
        node->internal_scope->token_end = token_end;

        parser.ribbon.commit();
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Scoped block parsed:\n%s\n", parser.ribbon.to_string().c_str());
        return node;
    }
    else
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Expecting close root_scope token\n");
    }

    graph_find_and_destroy_node(parser.graph, node);
    parser.ribbon.rollback();
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Scoped block parsed\n");
    return nullptr;
}

Node* parse_code_block(Parser_Context& parser, Scope* parent_scope, Node_Slot* flow_out)
{
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing code block...\n" );

    //
    // Parse n atomic code blocks
    //
    parser.ribbon.start_transaction();

    Node_Slot* last_node_flow_out  = flow_out;
    bool     block_end_reached = false;
    size_t   block_size        = 0;

    while (parser.ribbon.can_eat() && !block_end_reached )
    {
        if ( Node* current_block = parse_atomic_code_block( parser, parent_scope, last_node_flow_out) )
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
        parser.ribbon.commit();
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " parse code block:\n%s\n", parser.ribbon.to_string().c_str());
        return last_node_flow_out->node;
    }

    parser.ribbon.rollback();
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " parse code block. Block size is %llu\n", block_size );
    return nullptr;
}

Node_Slot* parse_expression(Parser_Context& parser, Scope* parent_scope, u8_t _precedence, Node_Slot* _left_override)
{
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing expression ...\n");

    /*
        Get the left-handed operand
    */
    Node_Slot* left = _left_override;

    if (!parser.ribbon.can_eat())
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Last token reached\n");
        return left;
    }

    if ( !left ) left = parse_parenthesis_expression(parser, parent_scope);
    if ( !left ) left = parse_unary_operation_expression(parser, parent_scope, _precedence);
    if ( !left ) left = parse_function_call(parser, parent_scope);
    if ( !left ) left = parse_variable_declaration(parser, parent_scope); // nullptr => variable won't be attached on the codeflow, it's a part of an expression..
    if ( !left ) left = parse_atomic_expression(parser, parent_scope);

    if (!parser.ribbon.can_eat())
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
    Node_Slot* expression_out = parse_binary_operation_expression(parser, parent_scope, _precedence, left );
    if ( expression_out )
    {
        if (!parser.ribbon.can_eat())
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Right side parsed, and last token reached\n");
            return expression_out;
        }
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Right side parsed, continue with a recursive call...\n");
        return parse_expression(parser, parent_scope, _precedence, expression_out);
    }

    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Returning left side only\n");

    return left;
}

bool _is_syntax_valid(const Parser_Context& parser)
{
    // TODO: optimization: is this function really useful ? It check only few things.
    //                     The parsing steps that follow (parseProgram) is doing a better check, by looking to what exist in the Parser.
    bool success = true;
    auto token = parser.ribbon.cbegin();
    short int opened = 0;

    while (token != parser.ribbon.cend() && success)
    {
        switch (token->type)
        {
            case Token_Type_parenthesis_open:
            {
                opened++;
                break;
            }
            case Token_Type_parenthesis_close:
            {
                if (opened <= 0)
                {
                    const size_t token_count = 10;
                    const size_t begin       = token->index < token_count ? 0 : token->index - token_count;
                    const size_t end         = token->index + 1;
                    NDBL_LOG(
                        Verbosity_Error,
                        "Parser",
                        "Syntax Error: Unexpected close bracket after \"... %s\" (position %llu)\n",
                        parser.ribbon.range_to_string(begin, end).c_str(),
                        token->char_position()
                    );
                    success = false;
                }
                opened--;
                break;
            }
            default:
                break;
        }

        std::advance(token, 1);
    }

    if (opened > 0)// same opened/closed parenthesis count required.
    {
        NDBL_LOG(Verbosity_Error, "Parser", "Syntax Error: Bracket count mismatch, %i still opened.\n", opened);
        success = false;
    }

    return success;
}

bool tokenize(Parser_Context& parser, const bdc::String& str)
{
    parser.buffer = str;
    parser.ribbon.reset( str );
    return tokenize(parser);
}

bool tokenize(Parser_Context& parser)
{
    NDBL_LOG(Verbosity_Diagnostic, "Parser", "Tokenization ...\n");

    bdc::String remainder = parser.buffer;
    size_t ignored_chars_count = 0;

    while ( !remainder.empty() )
    {
        Token  new_token = parse_token( parser,  remainder );

        if ( !new_token )
        {
            NDBL_LOG(
                Verbosity_Warning, "Parser", 
                NDBL_KO " Unable to tokenize from \"%20s...\" (at char %llu)\n", 
                remainder.c_str(), (u64_t)remainder.data - (u64_t)parser.buffer.data );
            return false;
        }

        // accumulate ignored chars (see else case to know why)
        if(new_token.type == Token_Type_ignore)
        {
            if ( parser.ribbon.empty() )
            {
                parser.ribbon.global_token.ltrim_word( new_token.size() );
                continue;
            }

            ignored_chars_count += new_token.size();
            continue;
        }

        if ( ignored_chars_count )
        {
            // case 1: if token type allows it => increase last token's prefix to wrap the ignored chars
            Token& back = parser.ribbon.back();
            if ( _accepts_suffix(parser, back.type) )
            {
                back.rextend_suffix(ignored_chars_count);
                NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "      \"%s\" (update) \n", back.view().c_str() );
            }
            // case 2: increase prefix of the new_token up to wrap the ignored chars
            else if ( new_token )
            {
                new_token.lextend_prefix(ignored_chars_count);
            }
            ignored_chars_count = 0;
        }

        parser.ribbon.push(new_token);
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "%4llu) \"%s\" \n", new_token.index, new_token.view().c_str() );
    }

    // Append remaining ignored chars to the ribbon's suffix
    if ( ignored_chars_count )
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Found ignored chars after tokenize, adding to the tokens suffix...\n");
        Token& tok = parser.ribbon.global_token;
        tok.rtrim_word( ignored_chars_count );
    }

    NDBL_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Tokenization.\n%s\n", parser.ribbon.to_string().c_str() );

    return true;
}

Token parse_token(const Parser_Context& parser, bdc::String& buffer)
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
        cursor += 1;
        
        String word = {
            buffer.data,
            cursor
        };

        buffer = {
            buffer.data + cursor,
            cursor > buffer.size ? 0 : buffer.size - cursor 
        };
        return Token{ Token_Type_ignore, word };
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
        cursor += 1;
        
        String word = {
            buffer.data,
            cursor
        };

        buffer = {
            buffer.data + cursor,
            cursor > buffer.size ? 0 : buffer.size - cursor 
        };
        return Token{ Token_Type_ignore, word };
    }

    // single-char
    auto single_char_found = hashmap_find(parser.langdef->token_type_by_single_char, buffer[0]); // index lookup
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
        auto keyword_found = hashmap_find(parser.langdef->token_type_by_keyword, word_hash.hash );
        if ( keyword_found )
        {            
            return Token{ *keyword_found.value, word };
        }

        // ...otherwise, symbol is an identifier
        return Token{ Token_Type_identifier, word};
        
    }
    return Token{ Token_Type_NULL };
}

Node_Slot* parse_function_call(Parser_Context& parser, Scope* parent_scope)
{
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "parse function call...\n");

    // Check if the minimum token count required is available ( 0: identifier, 1: open parenthesis, 2: close parenthesis)
    if (!parser.ribbon.can_eat(3))
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " 3 tokens min. are required\n");
        return nullptr;
    }

    parser.ribbon.start_transaction();

    // Try to parse regular function: function(...)
    bdc::String function_identifier;
    Token token_0 = parser.ribbon.eat();
    Token token_1 = parser.ribbon.eat();
    if (token_0.type == Token_Type_identifier &&
        token_1.type == Token_Type_parenthesis_open)
    {
        function_identifier = token_0.word_view();
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Regular function pattern detected.\n");
    }
    else // Try to parse operator like (ex: operator==(..,..))
    {
        Token token_2 = parser.ribbon.eat();// eat a "supposed open bracket>

        if (token_0.type == Token_Type_keyword_operator && token_1.type == Token_Type_operator && token_2.type == Token_Type_parenthesis_open)
        {
            function_identifier = token_1.word_view();// operator
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Operator function-like pattern detected.\n");
        }
        else
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Not a function.\n");
            parser.ribbon.rollback();
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
    while (!parsingError && parser.ribbon.can_eat() &&
        parser.ribbon.peek().type != Token_Type_parenthesis_close)
    {
        Node_Slot* expression_out = parse_expression(parser, parent_scope);
        if ( expression_out )
        {
            array_append(result_slots, expression_out );
            function_type.function_push_arg( expression_out->property->type );
            parser.ribbon.eat_if(Token_Type_list_separator);
        }
        else
        {
            parsingError = true;
        }
    }

    // eat "close bracket supposed" token
    if ( !parser.ribbon.eat_if(Token_Type_parenthesis_close) )
    {
        NDBL_LOG(Verbosity_Warning, "Parser", NDBL_KO " Expecting parenthesis close\n");
        parser.ribbon.rollback();
        return nullptr;
    }


    // Find the prototype in the parser library
    Node* fct_node = graph_create_function( parser.graph, &function_type, parent_scope );

    for ( int i = 0; i < fct_node->component.invokable.argument_slots.size; i++ )
    {
        // Connects each results to the corresponding input
        graph_connect_or_merge(result_slots[i], fct_node->component.invokable.argument_slots[i] );
    }

    parser.ribbon.commit();
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Function call parsed:\n%s\n", parser.ribbon.to_string().c_str() );

    return fct_node->value_out();
}

Node* parse_atomic_code_block(Parser_Context& parser, Scope* parent_scope, Node_Slot* flow_out)
{
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing atomic code block ..\n");
    ASSERT(flow_out);

    // most common case
    Node* block = nullptr;
            if ( (block = parse_scoped_block(parser, parent_scope, flow_out)) );
    else if ( (block = parse_return(parser, parent_scope, flow_out)));
    else if ( (block = parse_expression_block(parser, parent_scope, flow_out)) );
    else if ( (block = parse_if_block(parser, parent_scope, flow_out)) );
    else if ( (block = parse_for_block(parser, parent_scope, flow_out)) );
    else if ( (block = parse_while_block(parser, parent_scope, flow_out)) ) ;
    else      (block = parse_empty_block(parser, parent_scope, flow_out));

    if ( block )
    {
        if ( Token tok = parser.ribbon.eat_if(Token_Type_end_of_instruction) )
        {
            block->suffix = tok;
        }

        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Block found (class \"%s\")\n", block->get_class()->name.c_str() );
        return block;
    }

    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " No block found\n");
    return nullptr;
}

Node* parse_if_block(Parser_Context& parser, Scope* parent_scope, Node_Slot* flow_out)
{
    parser.ribbon.start_transaction();

    Token if_token = parser.ribbon.eat_if(Token_Type_keyword_if);
    if ( !if_token )
    {
        return nullptr;
    }

    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing if statement...\n");

    // if
    Node* if_node  = graph_create_cond_struct( parser.graph, parent_scope );
    if_node->component.branching.branch_prefix = parser.ribbon.get_eaten();

    graph_connect(flow_out, if_node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );

    if ( !parser.ribbon.eat_if(Token_Type_parenthesis_open) )
    {
        graph_find_and_destroy_node(parser.graph, if_node);
        parser.ribbon.rollback();
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Open bracket expected\n");
        return nullptr;
    }
    
    // if's condition
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing if block's condition...\n");
    parse_expression_block( parser, if_node->internal_scope, nullptr, if_node->component.branching.condition_in());

    if ( !parser.ribbon.eat_if(Token_Type_parenthesis_close) )
    {
        graph_find_and_destroy_node(parser.graph, if_node);
        parser.ribbon.rollback();
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Close bracket expected\n");
        return nullptr;
    }

    // if's block
    Node* if_block = parse_atomic_code_block( parser,  if_node->internal_scope, if_node->component.branching.branch_out(Branch_TRUE) );
    if( if_block )
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " if block parsed\n");
    }

    // else (optionnal)
    if ( parser.ribbon.eat_if(Token_Type_keyword_else) )
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing else statement ...\n");
        if_node->component.branching.branch_suffix = parser.ribbon.get_eaten();

        // else's block
        if ( Node* else_block = parse_atomic_code_block( parser,  if_node->internal_scope, if_node->component.branching.branch_out(Branch_FALSE) ) )
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " else block parsed.\n");
        }
        else
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " else block parsed (without code)\n");
        }
    }                   

    parser.ribbon.commit();
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_OK " Parse conditional structure:\n%s\n", parser.ribbon.to_string().c_str() );
    // TODO: connect true/false branches flow_out to scope flow_leave?"
    return if_node;
}

Node* parse_for_block(Parser_Context& parser, Scope* parent_scope, Node_Slot* flow_out)
{
    bool        success     = false;
    Node*    for_node    = nullptr;

    parser.ribbon.start_transaction();

    if ( Token token_for = parser.ribbon.eat_if(Token_Type_keyword_for) )
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing for loop ...\n");

        for_node = graph_create_for_loop( parser.graph, parent_scope );
        for_node->component.branching.branch_prefix = token_for;

        graph_connect( flow_out, for_node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );

        Token open_bracket = parser.ribbon.eat_if(Token_Type_parenthesis_open);
        if ( open_bracket)
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing for set_name/condition/iter instructions ...\n");

            // first we parse three instructions, no matter if we find them, we'll continue (we are parsing something abstract)

            // parse init; condition; iteration or nothing
            parse_expression_block(parser, for_node->internal_scope, nullptr, for_node->component.branching.initialization_slot)
            && parse_expression_block(parser, for_node->internal_scope, nullptr, for_node->component.branching.condition_in())
            && parse_expression_block(parser, for_node->internal_scope, nullptr, for_node->component.branching.iteration_slot);

            // parse parenthesis close
            if ( Token parenthesis_close = parser.ribbon.eat_if(Token_Type_parenthesis_close) )
            {
                Node* block = parse_atomic_code_block( parser,  for_node->internal_scope, for_node->component.branching.branch_out(Branch_TRUE) ) ;

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
        parser.ribbon.commit();
        // TODO: Should we connect true/false branches to scope's flow_leave Node_Slot?
        return for_node;
    }

    if ( for_node )
    {
        graph_find_and_destroy_node(parser.graph, for_node);
    }
    parser.ribbon.rollback();
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", NDBL_KO " Could not parse for block\n");
    return {};
}

Node* parse_while_block(Parser_Context& parser, Scope* parent_scope, Node_Slot* flow_out)
{
    bool        success     = false;
    Node*    while_node  = nullptr;
    Node*    block       = nullptr;

    parser.ribbon.start_transaction();

    if ( Token token_while = parser.ribbon.eat_if(Token_Type_keyword_while) )
    {
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing while ...\n");

        while_node = graph_create_while_loop( parser.graph, parent_scope );
        while_node->component.branching.branch_prefix = token_while;

        graph_connect( flow_out, while_node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );

        if ( Token open_bracket = parser.ribbon.eat_if(Token_Type_parenthesis_open) )
        {
            NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing while condition ... \n");

            // Parse an optional condition
            parse_expression_block( parser, while_node->internal_scope, nullptr, while_node->component.branching.condition_in());

            if (parser.ribbon.eat_if(Token_Type_parenthesis_close) )
            {
                block = parse_atomic_code_block( parser,  while_node->internal_scope, while_node->component.branching.branch_out(Branch_TRUE) );
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
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing while:\n%s\n", parser.ribbon.to_string().c_str() );
        parser.ribbon.commit();
        // TODO: Should we connect true/false branches to scope's flow_leave SLot?
        return while_node;
    }

    parser.ribbon.rollback();
    graph_find_and_destroy_node(parser.graph, while_node);
    graph_find_and_destroy_node(parser.graph, block);

    return {};
}

Node* parse_return(Parser_Context& parser, Scope* parent_scope, Node_Slot* flow_out)
{
    if (!parser.ribbon.can_eat(2))
    {
        return nullptr;
    }

    parser.ribbon.start_transaction();

    if ( Token return_token = parser.ribbon.eat_if(Token_Type_keyword_return) )
    {
        // Parse the expression at the right side of the return
        if ( Node_Slot* expression_out = parse_expression(parser, parent_scope) )
        {
            const Type_Descriptor* type = expression_out->property->type;
            Node* return_node = graph_create_return( parser.graph, type, parent_scope );
            return_node->value->token = return_token;

            // TODO: assign prefix and suffix to return Node

            // Connect the expression to the return Node
            graph_connect(expression_out, return_node->value_in());
            // and to the flow
            graph_connect(flow_out, return_node->flow_in());

            parser.ribbon.commit();
            return return_node;
        }
    }

    parser.ribbon.rollback();
    return nullptr;
}

Node_Slot* parse_variable_declaration(Parser_Context& parser, Scope* parent_scope)
{
    if (!parser.ribbon.can_eat(2))
    {
        return nullptr;
    }

    parser.ribbon.start_transaction();

    bool  success          = false;
    Token type_token       = parser.ribbon.eat();
    Token identifier_token = parser.ribbon.eat();

    if (type_token.is_keyword_type() && identifier_token.type == Token_Type_identifier)
    {
        const Type_Descriptor* type = langdef_get_type_descriptor_from_token_type(*parser.langdef, type_token.type);
        Node* variable_node = graph_create_variable( parser.graph, type, identifier_token.word_view(), parent_scope );

        SET_FLAGS(variable_node->component.variable.flags, VariableFlag_DECLARED);
        variable_node->component.variable.type_token = type_token;
        node_set_identifier_token(variable_node, identifier_token );

        // declaration with assignment ?
        Token operator_token = parser.ribbon.eat_if(Token_Type_operator);
        if (operator_token && operator_token.word_view() == "=")
        {
            // an expression is expected
            if ( Node_Slot* expression_out = parse_expression(parser, parent_scope) )
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
            parser.ribbon.commit();
            return variable_node->value_out();
        }

        NDBL_DEBUG_LOG(
            Verbosity_Diagnostic, "Parser", 
            NDBL_KO "  Initialization expression expected for %s\n", identifier_token.word_view().c_str());
        graph_find_and_destroy_node(parser.graph, variable_node);
    }

    parser.ribbon.rollback();
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------------
//
//                                  SERIALIZATION
//
//---------------------------------------------------------------------------------------------------------------------------

const Node_Slot* serialize_invokable(Parser_Context& parser, const Node* _node)
{
    if (_node->type == Node_Type_OPERATOR )
    {
        Array<Node_Slot*> args = array_view( _node->component.invokable.argument_slots );
        int precedence = langdef_get_precedence(*parser.langdef, &_node->component.invokable.type);

        switch ( _node->component.invokable.type.function.args.size )
        {
            case 2:
            {
                // Left part of the expression
                {
                    const Type_Descriptor* l_func_type = node_get_connected_function_type(_node, LEFT_VALUE_PROPERTY);
                    bool needs_braces = l_func_type && langdef_get_precedence(*parser.langdef, l_func_type) < precedence;
                    Serialization_Flags flags = Serialization_Flag_RECURSE
                                        | needs_braces * Serialization_Flag_WRAP_WITH_BRACES ;
                    serialize_input( parser, args[0], flags );
                }

                // Operator
                VERIFY( _node->component.invokable.identifier_token, "identifier token should have been assigned in parse_function_call");
                string_builder_append( parser.sb, serialize_token( parser, _node->component.invokable.identifier_token ));

                // Right part of the expression
                {
                    const Type_Descriptor* r_func_type = node_get_connected_function_type(_node, RIGHT_VALUE_PROPERTY);
                    bool needs_braces = r_func_type && langdef_get_precedence(*parser.langdef, r_func_type) < precedence;
                    Serialization_Flags flags = Serialization_Flag_RECURSE
                                        | needs_braces * Serialization_Flag_WRAP_WITH_BRACES ;
                    serialize_input( parser, args[1], flags );
                }
                break;
            }

            case 1:
            {
                // operator ( ... innerOperator ... )   ex:   -(a+b)

                ASSERT( _node->component.invokable.identifier_token );
                string_builder_append( parser.sb, serialize_token( parser, _node->component.invokable.identifier_token) );

                bool needs_braces    = node_get_connected_function_type(_node, LEFT_VALUE_PROPERTY) != nullptr;
                Serialization_Flags flags = Serialization_Flag_RECURSE
                                    | needs_braces * Serialization_Flag_WRAP_WITH_BRACES;
                serialize_input( parser, args[0], flags );
                break;
            }
        }
    }
    else
    {
        serialize_function_call(parser, &_node->component.invokable.type, array_view(_node->component.invokable.argument_slots) );
    }

    return _node->value_out();
}

void serialize_function_call(Parser_Context& parser, const Type_Descriptor *function_type, const bdc::Array<Node_Slot*>& inputs)
{
    string_builder_append( parser.sb, function_type->name );
    
    string_builder_append( parser.sb, serialize_token_type(parser, Token_Type_parenthesis_open));

    for (const Node_Slot* input_slot : inputs)
    {
        ASSERT( HAS_FLAGS(input_slot->flags, Node_Slot::Flag_INPUT) );
        if ( input_slot != inputs[0])
        {
            string_builder_append( parser.sb, serialize_token_type(parser, Token_Type_list_separator));
        }
        serialize_input( parser, input_slot, Serialization_Flag_RECURSE );
    }

    string_builder_append( parser.sb, serialize_token_type(parser, Token_Type_parenthesis_close) );
}

void serialize_function_type(Parser_Context& parser, const Type_Descriptor *function_type)
{
    string_builder_append( parser.sb, serialize_type(parser, function_type->function.return_type));
    string_builder_append( parser.sb, " ");
    string_builder_append( parser.sb, function_type->name );
    string_builder_append( parser.sb, serialize_token_type(parser, Token_Type_parenthesis_open) );

    for (auto it = function_type->function.args.begin(); it != function_type->function.args.end(); it++)
    {
        if (it != function_type->function.args.begin())
        {
            string_builder_append( parser.sb, serialize_token_type(parser, Token_Type_list_separator));
            string_builder_append( parser.sb, " ");
        }
        string_builder_append( parser.sb, serialize_type(parser, (*it).type) );
    }

    string_builder_append( parser.sb, serialize_token_type(parser, Token_Type_parenthesis_close));
}

void serialize_variable_ref(Parser_Context& parser, const Node* _node)
{
    ASSERT(_node->type == Node_Type_VARIABLE_REF);
    String token_str = serialize_token( parser, node_get_identifier_token(_node) );
    string_builder_append( parser.sb, token_str);
}

void serialize_variable(Parser_Context& parser, const Node *_node)
{
    ASSERT(_node->type == Node_Type_VARIABLE);

    // 1. Serialize variable's type

    // If parsed
    if ( _node->component.variable.type_token )
    {
        string_builder_append(parser.sb, serialize_token( parser, _node->component.variable.type_token) );
    }
    else // If created in the graph by the user
    {
        string_builder_append(parser.sb, serialize_type(parser, _node->value->type) );
        string_builder_append(parser.sb, " ");
    }

    // 2. Serialize variable identifier
    string_builder_append(parser.sb, serialize_token( parser, node_get_identifier_token(_node) ));

    // 3. Initialisation
    //    When a VariableNode has its input connected, we serialize it as its initialisation expression

    const Node_Slot* slot = _node->value_in();
    if ( slot->adjacent.size != 0 )
    {
        if ( _node->component.variable.operator_token )
            string_builder_append(parser.sb, _node->component.variable.operator_token.view());
        else
            string_builder_append(parser.sb, " = ");

        serialize_input( parser, slot, Serialization_Flag_RECURSE );
    }
}

void serialize_return(Parser_Context& parser, const Node* node)
{
    ASSERT(node->type == Node_Type_RETURN);

    if( node->value->token )
    {
        string_builder_append(parser.sb, serialize_token( parser, node->value->token ));
    }
    else
    {
        string_builder_append(parser.sb, *hashmap_find(parser.langdef->keyword_by_token_type, Token_Type_keyword_return).value );
        string_builder_append(parser.sb, " ");
    }

    if ( const Node_Slot* input_slot = node->value_in() )
    {
        serialize_input( parser, input_slot, Serialization_Flag_RECURSE );
    }
}

void serialize_input(Parser_Context& parser, const Node_Slot* slot, Serialization_Flags _flags )
{
    ASSERT( HAS_FLAGS(slot->flags, Node_Slot::Flag_INPUT ) );

    const Node_Slot*     adjacent_slot     = slot->first_adjacent();
    const Node_Property* adjacent_property = adjacent_slot != nullptr ? adjacent_slot->property
                                                                        : nullptr;
    // Append open brace?
    if ( _flags & Serialization_Flag_WRAP_WITH_BRACES )
        string_builder_append(parser.sb, serialize_token_type(parser,  Token_Type_parenthesis_open));

    if ( adjacent_property == nullptr )
    {
        // Simply serialize this property
        serialize_property(parser, slot->property);
    }
    else
    {
        VERIFY( _flags & Serialization_Flag_RECURSE, "Why would you call serialize_input without RECURSE flag?");
        // Append token prefix?
        if (adjacent_property->token)
            string_builder_append(parser.sb, adjacent_property->token.prefix_view());

        // Serialize adjacent slot
        serialize_node_value_out(parser, adjacent_slot, Serialization_Flag_RECURSE);

        // Append token suffix?
        if (adjacent_property->token )
                string_builder_append(parser.sb, adjacent_property->token.suffix_view());
    }

    // Append close brace?
    if ( _flags & Serialization_Flag_WRAP_WITH_BRACES )
        string_builder_append( parser.sb, serialize_token_type(parser, Token_Type_parenthesis_close));
}

void serialize_node_value_out(Parser_Context& parser, const Node_Slot* slot, Serialization_Flags _flags)
{
    // If output is node's output value, we serialize the node
    if( slot == slot->node->value_out() )
    {
        return serialize_node(parser, slot->node, _flags);
        return;
    }

    // Otherwise, it might be a variable reference, so we serialize the identifier only
    ASSERT(slot->node->type == Node_Type_VARIABLE ); // Can't be another type
    VERIFY( slot == slot->node->component.variable.ref_out, "Cannot serialize an other slot from a VariableNode");
    string_builder_append(  parser.sb, node_get_identifier(slot->node) );
}

void serialize_node(Parser_Context& parser, const Node* node, Serialization_Flags _flags )
{
    if ( node == nullptr )
        return;

    ASSERT( _flags == Serialization_Flag_RECURSE ); // The only flag configuration handled for now

    switch ( node->type )
    {
        case Node_Type_RETURN:            serialize_return(parser, node);                   break;
        case Node_Type_IF_ELSE:           serialize_if_else(parser, node);                  break;
        case Node_Type_FOR_LOOP:          serialize_for_loop(parser, node);                 break;
        case Node_Type_WHILE_LOOP:        serialize_while_loop(parser, node);               break;
        case Node_Type_LITERAL:           serialize_literal(parser, node);                  break;
        case Node_Type_VARIABLE:          serialize_variable(parser, node);                 break;
        case Node_Type_VARIABLE_REF:      serialize_variable_ref(parser, node);             break;
        case Node_Type_FUNCTION:          [[fallthrough]];        
        case Node_Type_OPERATOR:          serialize_invokable(parser, node);                break;
        case Node_Type_EMPTY_INSTRUCTION: serialize_empty_instruction(parser, node);        break;
        case Node_Type_ROOT:              [[fallthrough]];
        case Node_Type_SCOPE:             serialize_scope(parser, node->internal_scope );   break;
        default:                          VERIFY(false, "Unhandled NodeType, can't serialize");
    }

    String token_str = serialize_token( parser, node->suffix );
    string_builder_append( parser.sb, token_str);
}

void serialize_scope(Parser_Context& parser, const Scope* scope)
{
    string_builder_append( parser.sb, serialize_token( parser, scope->token_begin) );
    
    for(Node* node : scope_get_backbone(scope) )
    {
        serialize_node( parser, node, Serialization_Flag_RECURSE);
    }
    
    string_builder_append( parser.sb, serialize_token( parser, scope->token_end) );
}

bdc::String serialize_bool(const Parser_Context& parser, bool b)
{
    return b ? "true" : "false";
}

bdc::String serialize_int(const Parser_Context& parser, int i)
{
    return string_printf( "%i", i );
}

bdc::String serialize_double(const Parser_Context& parser, double d)
{
    return string_printf( "%d", d );
}

bdc::String serialize_token(const Parser_Context& parser, const Token& token)
{
    if ( token.type == Token_Type_NULL )
        return {};

    return token.view();
}

bdc::String serialize_token_type(const Parser_Context& parser, Token_Type token_type)
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
            if (auto found = hashmap_find(parser.langdef->keyword_by_token_type, token_type))
            {
                return *found.value;
            }
            if (auto found = hashmap_find(parser.langdef->single_char_by_keyword, token_type))
            {
                return String{*found.value};
            }
            return "<?>";
        }
    }
}

void serialize_graph(Parser_Context& parser, const Graph* graph )
{
    const Node* root_node = graph_root(graph);
    if ( root_node == nullptr )
    {
        NDBL_LOG(Verbosity_Error, "Serializer", "a root primary_child is expected to serialize the graph\n");
        return;
    }
    serialize_node(parser, root_node, Serialization_Flag_RECURSE);
}

void serialize_for_loop(Parser_Context& parser, const Node* _for_loop)
{
    ASSERT( _for_loop->type == Node_Type_FOR_LOOP );

    string_builder_append( parser.sb, serialize_token( parser, _for_loop->component.branching.branch_prefix) );
    string_builder_append( parser.sb, serialize_token_type(parser, Token_Type_parenthesis_open) );
    {
        const Node_Slot* init_slot = node_find_slot_by_property_name(_for_loop, INITIALIZATION_PROPERTY, Node_Slot::Flag_INPUT );
        const Node_Slot* cond_slot = node_find_slot_by_property_name(_for_loop, CONDITION_PROPERTY, Node_Slot::Flag_INPUT );
        const Node_Slot* iter_slot = node_find_slot_by_property_name(_for_loop, ITERATION_PROPERTY, Node_Slot::Flag_INPUT );
        serialize_input( parser, init_slot, Serialization_Flag_RECURSE );
        serialize_input( parser, cond_slot, Serialization_Flag_RECURSE );
        serialize_input( parser, iter_slot, Serialization_Flag_RECURSE );
    }
    string_builder_append( parser.sb, serialize_token_type(parser, Token_Type_parenthesis_close) );
    serialize_node( parser, _for_loop->component.branching.branch_out(Branch_TRUE)->first_adjacent_node(), Serialization_Flag_RECURSE );
}

void serialize_while_loop(Parser_Context& parser, const Node* _while_loop_node)
{
    ASSERT( _while_loop_node->type == Node_Type_WHILE_LOOP );

    // while
    String while_str = serialize_token( parser, _while_loop_node->component.branching.branch_prefix);
    string_builder_append(parser.sb, while_str);

    // condition
    Serialization_Flags flags = Serialization_Flag_RECURSE
                        | Serialization_Flag_WRAP_WITH_BRACES;
    serialize_input( parser, _while_loop_node->component.branching.condition_in(), flags );

    if ( const Node* _node = _while_loop_node->component.branching.branch_out(Branch_TRUE)->first_adjacent_node() )
    {
        serialize_node( parser, _node, Serialization_Flag_RECURSE);
    }
}

void serialize_if_else(Parser_Context& parser, const Node* if_node )
{
    ASSERT( if_node->type == Node_Type_IF_ELSE );

    // if
    String if_str = serialize_token( parser, if_node->component.branching.branch_prefix );
    string_builder_append(parser.sb,  if_str );

    // condition
    Serialization_Flags flags = Serialization_Flag_RECURSE
                        | Serialization_Flag_WRAP_WITH_BRACES;
    serialize_input(parser, if_node->component.branching.condition_in(), flags );

    // when condition is true
    serialize_node(parser, if_node->component.branching.branch_out(Branch_TRUE)->first_adjacent_node(), Serialization_Flag_RECURSE );

    // when condition is false
    string_builder_append(parser.sb, serialize_token( parser, if_node->component.branching.branch_suffix) );
    serialize_node(parser, if_node->component.branching.branch_out(Branch_FALSE)->first_adjacent_node(), Serialization_Flag_RECURSE );
}

void serialize_property(Parser_Context& parser, const Node_Property* property)
{
    const String property_str = serialize_token( parser, property->token);
    string_builder_append( parser.sb, property_str );
}

bdc::String serialize_type(const Parser_Context& parser, const Type_Descriptor* type)
{
    if (auto found = hashmap_find(parser.langdef->keyword_by_type_id, type->id.hash_code() ))
    {
        return *found.value;
    }
    return "";
}

void serialize_literal(Parser_Context& parser, const Node* node)
{
    ASSERT( node->type == Node_Type_LITERAL );
    serialize_property( parser, node->value );
}

void serialize_empty_instruction(Parser_Context& parser, const Node* node)
{
    ASSERT( node->type == Node_Type_EMPTY_INSTRUCTION );
    string_builder_append( parser.sb, serialize_token( parser, node->value->token ) );
}

String parser_build_tstring(Parser_Context& parser)
{
    String result = string_builder_build_tstring(parser.sb);
    return result;
}

String parser_build_string(Parser_Context& parser)
{
    String result = string_builder_build_string(parser.sb);
    return result;
}

} // namespace ndbl