//---------------------------------------------------------------------------------------------------------------------------
// Nodlang.cpp
// This file is structured in 3 parts, use Ctrl + F to search:
//  [SECTION] A. Declaration (types, keywords, etc.)
//  [SECTION] B. Parser
//  [SECTION] C. Serializer
//---------------------------------------------------------------------------------------------------------------------------

#include "Nodlang.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <sstream>
#include <string>
#include <chrono>
#include <cctype> // isdigit, isalpha, and isalnum.

#include "tools/core/reflection/reflection"
#include "tools/core/format.h"
#include "tools/core/log.h"
#include "tools/core/hash.h"
#include "tools/core/memory/memory.h"

#include "ndbl/core/Utils.h"
#include "ndbl/core/DirectedEdge.h"
#include "ndbl/core/ForLoopNode.h"
#include "ndbl/core/Graph.h"
#include "ndbl/core/IfNode.h"
#include "ndbl/core/FunctionNode.h"
#include "ndbl/core/LiteralNode.h"
#include "ndbl/core/Property.h"
#include "ndbl/core/Scope.h"
#include "ndbl/core/VariableNode.h"
#include "ndbl/core/VariableRefNode.h"
#include "ndbl/core/WhileLoopNode.h"
#include "ndbl/core/language/Nodlang_biology.h"
#include "ndbl/core/language/Nodlang_math.h"

using namespace ndbl;
using namespace tools;

static Nodlang* g_language{ nullptr };

//---------------------------------------------------------------------------------------------------------------------------
// [SECTION] A. Declaration -------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------

Nodlang::Nodlang(bool _strict)
    : m_strict_mode(_strict)
    , parser_state()
{
    // A.1. Define the language
    //-------------------------
    m_definition.chars =
    {
        { '(',    Token_t::parenthesis_open},
        { ')',    Token_t::parenthesis_close},
        { '{',    Token_t::scope_begin},
        { '}',    Token_t::scope_end},
        { '\n',   Token_t::ignore},
        { '\t',   Token_t::ignore},
        { ' ',    Token_t::ignore},
        { ';',    Token_t::end_of_instruction},
        { ',',    Token_t::list_separator}
    };

    m_definition.keywords =
    {
         { "if",       Token_t::keyword_if },
         { "for",      Token_t::keyword_for },
         { "while",    Token_t::keyword_while },
         { "else",     Token_t::keyword_else },
         { "true",     Token_t::literal_bool },
         { "false",    Token_t::literal_bool },
         { "operator", Token_t::keyword_operator },
    };

    m_definition.types =
    {
         { "bool",   Token_t::keyword_bool,   type::get<bool>()},
         { "string", Token_t::keyword_string, type::get<std::string>()},
         { "double", Token_t::keyword_double, type::get<double>()},
         { "i16",    Token_t::keyword_i16,    type::get<i16_t>()},
         { "int",    Token_t::keyword_int,    type::get<i32_t>()},
         { "any",    Token_t::keyword_any,    type::get<any>()},
         // we don't really want to parse/serialize that
         // { "unknown",Token_t::keyword_unknown,type::get<unknown>()},
    };

    m_definition.operators =
    {
         {"-",   Operator_t::Unary,   5},
         {"!",   Operator_t::Unary,   5},
         {"/",   Operator_t::Binary, 20},
         {"*",   Operator_t::Binary, 20},
         {"+",   Operator_t::Binary, 10},
         {"-",   Operator_t::Binary, 10},
         {"||",  Operator_t::Binary, 10},
         {"&&",  Operator_t::Binary, 10},
         {">=",  Operator_t::Binary, 10},
         {"<=",  Operator_t::Binary, 10},
         {"=>",  Operator_t::Binary, 10},
         {"==",  Operator_t::Binary, 10},
         {"<=>", Operator_t::Binary, 10},
         {"!=",  Operator_t::Binary, 10},
         {">",   Operator_t::Binary, 10},
         {"<",   Operator_t::Binary, 10},
         {"=",   Operator_t::Binary,  0},
         {"+=",  Operator_t::Binary,  0},
         {"-=",  Operator_t::Binary,  0},
         {"/=",  Operator_t::Binary,  0},
         {"*=",  Operator_t::Binary,  0}
    };

    // A.2. Create indexes
    //---------------------
    for( auto [_char, token_t] : m_definition.chars)
    {
        m_token_t_by_single_char.insert({_char, token_t});
        m_single_char_by_keyword.insert({token_t, _char});
    }

    for( auto [keyword, token_t] : m_definition.keywords)
    {
        m_token_t_by_keyword.insert({hash::hash_cstr(keyword), token_t});
        m_keyword_by_token_t.insert({token_t, keyword});
    }

    for( auto [keyword, token_t, type] : m_definition.types)
    {
        m_keyword_by_token_t.insert({token_t, keyword});
        m_keyword_by_type_id.insert({type->id(), keyword});
        m_token_t_by_keyword.insert({hash::hash_cstr(keyword), token_t});
        m_token_t_by_type_id.insert({type->id(), token_t});
        m_type_by_token_t.insert({token_t, type});
    }

    for( auto [keyword, operator_t, precedence] : m_definition.operators)
    {
        const Operator *op = new Operator(keyword, operator_t, precedence);
        ASSERT(std::find(m_operators.begin(), m_operators.end(), op) == m_operators.end());
        m_operators.push_back(op);
    }

    // A.3. Load libraries
    //---------------------

    load_library<Nodlang_math>();     // contains all operator implementations
    load_library<Nodlang_biology>();  // a function to convert RNA (library is wip)
}

Nodlang::~Nodlang()
{
    for(const Operator* each : m_operators )
        delete each;

//    for(const IInvokable* each : m_functions ) // static and member functions are owned by their respective tools::type<T>
//        delete each;
}
//---------------------------------------------------------------------------------------------------------------------------
// [SECTION] B. Parser ------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------

bool Nodlang::parse(Graph* graph_out, const std::string& code)
{
    parser_state.reset_scope_stack();
    parser_state.reset_graph( graph_out );
    parser_state.reset_ribbon(code.c_str(), code.size());

    LOG_VERBOSE("Parser", "Parsing ...\n%s\n", code.c_str());

    if ( !tokenize() )
    {
        return false;
    }

    if (!is_syntax_valid())
    {
        return false;
    }

    Optional<Node*> root_node = parse_program();

    if ( !root_node )
        return false;

    if (parser_state.tokens().can_eat() )
    {
        parser_state.graph()->clear();
        LOG_VERBOSE("Parser", KO "End of token ribbon expected\n");
        LOG_VERBOSE("Parser", "%s", format::title("TokenRibbon").c_str());
        for (const Token& each_token : parser_state.tokens() )
        {
            LOG_VERBOSE("Parser", "token idx %i: %s\n", each_token.m_index, each_token.json().c_str());
        }
        LOG_VERBOSE("Parser", "%s", format::title("TokenRibbon end").c_str());
        auto curr_token = parser_state.tokens().peek();
        LOG_VERBOSE("Parser", KO "Failed to parse from token %llu/%llu and above.\n", curr_token.m_index, parser_state.tokens().size());
        LOG_ERROR("Parser", "Unable to parse all the tokens\n");
        return false;
    }
    return true;
}

bool Nodlang::parse_bool_or(const std::string &_str, bool default_value) const
{
    size_t cursor = 0;
    Token  token  = parse_token(_str.c_str(), _str.size(), cursor);
    if ( token.m_type == Token_t::literal_bool )
        return _str == std::string("true");
    return default_value;
}

std::string Nodlang::remove_quotes(const std::string &_quoted_str) const
{
    ASSERT(_quoted_str.size() >= 2);
    ASSERT(_quoted_str.front() == '\"');
    ASSERT(_quoted_str.back() == '\"');
    return std::string(++_quoted_str.cbegin(), --_quoted_str.cend());
}

double Nodlang::parse_double_or(const std::string &_str, double default_value) const
{
    size_t cursor = 0;
    Token  token  = parse_token(_str.c_str(), _str.size(), cursor);
    if ( token.m_type == Token_t::literal_double )
        return std::stod(_str);
    return default_value;
}


int Nodlang::parse_int_or(const std::string &_str, int default_value) const
{
    size_t cursor = 0;
    Token  token  = parse_token(_str.c_str(), _str.size(), cursor);
    if ( token.m_type == Token_t::literal_int )
        return stoi(_str);
    return default_value;
}

Optional<Slot*> Nodlang::token_to_slot(Token _token)
{
    if (_token.m_type == Token_t::identifier)
    {
        std::string identifier = _token.word_to_string();

        Scope* scope = parser_state.current_scope();
        VERIFY(scope, "No current scope, at least one base scope is required in the scope stack");
        if( VariableNode* existing_variable = scope->find_var( identifier ) )
        {
            return existing_variable->ref_out();
        }

        if ( !m_strict_mode )
        {
            // Insert a VariableNodeRef with "any" type
            LOG_WARNING( "Parser", "%s is not declared (strict mode), abstract graph can be generated but compilation will fail.\n",
                         _token.word_to_string().c_str() );
            VariableRefNode* ref = parser_state.graph()->create_variable_ref();
            ref->value()->set_token(_token );
            return ref->value_out();
        }

        LOG_ERROR( "Parser", "%s is not declared (strict mode) \n", _token.word_to_string().c_str() );
        return nullptr;
    }

    LiteralNode* literal{};

    switch (_token.m_type)
    {
        case Token_t::literal_bool:   literal = parser_state.graph()->create_literal<bool>();        break;
        case Token_t::literal_int:    literal = parser_state.graph()->create_literal<i32_t>();       break;
        case Token_t::literal_double: literal = parser_state.graph()->create_literal<double>();      break;
        case Token_t::literal_string: literal = parser_state.graph()->create_literal<std::string>(); break;
        default:
            break; // we don't want to throw
    }

    if ( literal )
    {
        LOG_VERBOSE("Parser", OK "Token %s converted to a Literal %s\n",
                    _token.word_to_string().c_str(),
                    literal->value()->get_type()->get_name());
        literal->value()->set_token( _token );
        return literal->value_out();
    }

    LOG_VERBOSE("Parser", KO "Unable to run token_to_slot with token %s!\n", _token.word_to_string().c_str());
    return nullptr;
}

Optional<Slot*> Nodlang::parse_binary_operator_expression(u8_t _precedence, Slot* _left)
{
    LOG_VERBOSE("Parser", "Parsing binary expression ...\n");
    ASSERT(_left != nullptr);

    if (!parser_state.tokens().can_eat(2))
    {
        LOG_VERBOSE("Parser", KO "Not enough tokens\n");
        return nullptr;
    }

    parser_state.start_transaction();
    const Token operator_token = parser_state.tokens().eat();
    const Token operand_token  = parser_state.tokens().peek();

    // Structure check
    const bool isValid = operator_token.m_type == Token_t::operator_ &&
                         operand_token.m_type != Token_t::operator_;

    if (!isValid)
    {
        parser_state.rollback();
        LOG_VERBOSE("Parser", KO "Unexpected tokens\n");
        return nullptr;
    }

    std::string word = operator_token.word_to_string();  // FIXME: avoid std::string copy, use hash
    const Operator *ope = find_operator(word, Operator_t::Binary);
    if (ope == nullptr)
    {
        LOG_VERBOSE("Parser", KO "Operator %s not found\n", word.c_str());
        parser_state.rollback();
        return nullptr;
    }

    // Precedence check
    if (ope->precedence <= _precedence && _precedence > 0)
    {// always update the first operation if they have the same precedence or less.
        LOG_VERBOSE("Parser", KO "Has lower precedence\n");
        parser_state.rollback();
        return nullptr;
    }

    // Parse right expression
    if ( Optional<Slot*> right = parse_expression(ope->precedence) )
    {
        // Create a function signature according to ltype, rtype and operator word
        FunctionDescriptor* type = FunctionDescriptor::create<any()>(ope->identifier.c_str());
        type->push_arg( _left->property->get_type());
        type->push_arg(right->property->get_type());

        FunctionNode* binary_op = parser_state.graph()->create_operator(type);
        binary_op->set_identifier_token( operator_token );
        binary_op->lvalue_in()->property->token().m_type = _left->property->token().m_type;
        binary_op->rvalue_in()->property->token().m_type = right->property->token().m_type;

        parser_state.graph()->connect_or_merge( _left         , binary_op->lvalue_in());
        parser_state.graph()->connect_or_merge( right.get() , binary_op->rvalue_in() );

        parser_state.commit();
        LOG_VERBOSE("Parser", OK "Binary expression parsed:\n%s\n", parser_state.tokens().to_string().c_str());
        return binary_op->value_out();
    }

    LOG_VERBOSE("Parser", KO "Right expression is null\n");
    parser_state.rollback();
    return nullptr;
}

Optional<Slot*> Nodlang::parse_unary_operator_expression(u8_t _precedence)
{
    LOG_VERBOSE("Parser", "parseUnaryOperationExpression...\n");

    if (!parser_state.tokens().can_eat(2))
    {
        LOG_VERBOSE("Parser", KO "Not enough tokens\n");
        return nullptr;
    }

    parser_state.start_transaction();
    Token operator_token = parser_state.tokens().eat();

    // Check if we get an operator first
    if (operator_token.m_type != Token_t::operator_)
    {
        parser_state.rollback();
        LOG_VERBOSE("Parser", KO "Expecting an operator token first\n");
        return nullptr;
    }

    // Parse expression after the operator
    Optional<Slot*> out_atomic = parse_atomic_expression();

    if ( !out_atomic )
    {
        out_atomic = parse_parenthesis_expression();
    }

    if ( !out_atomic )
    {
        LOG_VERBOSE("Parser", KO "Right expression is null\n");
        parser_state.rollback();
        return nullptr;
    }

    // Create a function signature
    FunctionDescriptor* type = FunctionDescriptor::create<any()>(operator_token.word_to_string().c_str());
    type->push_arg( out_atomic->property->get_type());

    FunctionNode* node = parser_state.graph()->create_operator(type);
    node->set_identifier_token( operator_token );
    node->lvalue_in()->property->token().m_type = out_atomic->property->token().m_type;

    parser_state.graph()->connect_or_merge( out_atomic.get(), node->lvalue_in() );

    LOG_VERBOSE("Parser", OK "Unary expression parsed:\n%s\n", parser_state.tokens().to_string().c_str());
    parser_state.commit();

    return node->value_out();
}

Optional<Slot*> Nodlang::parse_atomic_expression()
{
    LOG_VERBOSE("Parser", "Parsing atomic expression ... \n");

    if (!parser_state.tokens().can_eat())
    {
        LOG_VERBOSE("Parser", KO "Not enough tokens\n");
        return nullptr;
    }

    parser_state.start_transaction();
    Token token = parser_state.tokens().eat();

    if (token.m_type == Token_t::operator_)
    {
        LOG_VERBOSE("Parser", KO "Cannot start with an operator token\n");
        parser_state.rollback();
        return nullptr;
    }

    if ( Optional<Slot*> result = token_to_slot(token) )
    {
        parser_state.commit();
        LOG_VERBOSE("Parser", OK "Atomic expression parsed:\n%s\n", parser_state.tokens().to_string().c_str());
        return result;
    }

    parser_state.rollback();
    LOG_VERBOSE( "Parser", KO "Unable to parse token (%llu)\n", token.m_index );

    return nullptr;
}

Optional<Slot*> Nodlang::parse_parenthesis_expression()
{
    LOG_VERBOSE("Parser", "parse parenthesis expr...\n");

    if (!parser_state.tokens().can_eat())
    {
        LOG_VERBOSE("Parser", KO "No enough tokens.\n");
        return nullptr;
    }

    parser_state.start_transaction();
    Token currentToken = parser_state.tokens().eat();
    if (currentToken.m_type != Token_t::parenthesis_open)
    {
        LOG_VERBOSE("Parser", KO "Open bracket not found.\n");
        parser_state.rollback();
        return nullptr;
    }

    Optional<Slot*> result = parse_expression();
    if ( result )
    {
        Token token = parser_state.tokens().eat();
        if (token.m_type != Token_t::parenthesis_close)
        {
            LOG_VERBOSE("Parser", "%s \n", parser_state.tokens().to_string().c_str());
            LOG_VERBOSE("Parser", KO "Parenthesis close expected\n",
                        token.word_to_string().c_str());
            parser_state.rollback();
        }
        else
        {
            LOG_VERBOSE("Parser", OK "Parenthesis expression parsed:\n%s\n", parser_state.tokens().to_string().c_str());
            parser_state.commit();
        }
    }
    else
    {
        LOG_VERBOSE("Parser", KO "No expression after open parenthesis.\n");
        parser_state.rollback();
    }
    return result;
}

Optional<Node*> Nodlang::parse_expression_block(Slot* input )
{
    parser_state.start_transaction();
    Optional<Slot*> value_out = parse_expression();

    if ( value_out )
    {
        // Special case: substitute variable by a reference
        if (value_out->node->type() == NodeType_VARIABLE )
        {
            auto variable = static_cast<VariableNode*>( value_out->node );
            if ( Utils::is_connected_to_codeflow(variable) ) // in such case, we have to reference the variable, since a given variable can't be twice (be declared twice) in the codeflow
            {
                VariableRefNode* ref = parser_state.graph()->create_variable_ref();
                ref->set_variable( variable );

                value_out = ref->value_out();
            }
        }
    }

    // Handle suffix
    if (parser_state.tokens().can_eat())
    {
        if ( parser_state.tokens().peek( Token_t::end_of_instruction ) )
        {
            LOG_VERBOSE("Parser", "End of instruction token found.\n" );
        }
        else if ( parser_state.tokens().peek( Token_t::parenthesis_close ) )
        {
            LOG_VERBOSE("Parser", "Parenthesis close detected.\n");
        }
        else
        {
            LOG_VERBOSE("Parser", "Parenthesis close expected.\n");
            value_out.reset();
        }
    }
    else
    {
        // we accept  to end like "... expression end>EOF"
    }

    if ( !value_out && input )
    {
        if (Token tok = parser_state.tokens().eat_if(Token_t::end_of_instruction))
        {
            input->property->set_token( tok );
            parser_state.commit();
            LOG_VERBOSE("Parser", OK "parse empty instruction\n");
            return input->node;
        }
    }

    if ( value_out )
    {
        Node* expression_node = value_out->node;
        if ( input )
        {
            // value_out ---> input
            parser_state.graph()->connect(value_out.data(),
                                          input,
                                          ConnectFlag_ALLOW_SIDE_EFFECTS);
            parser_state.commit();
        }
        LOG_VERBOSE("Parser", OK "parse instruction:\n%s\n", parser_state.tokens().to_string().c_str());
        return expression_node;
    }

    parser_state.rollback();
    LOG_VERBOSE("Parser", KO "parse instruction\n");
    return nullptr;
}

Optional<Node*> Nodlang::parse_program()
{
    VERIFY( parser_state.graph() != nullptr, "A Graph is expected");

    parser_state.start_transaction();

    // Create an entry point and push its scope
    Node* entry_point = parser_state.graph()->create_entry_point();

    // To preserve any ignored characters stored in the global token
    // we put the prefix and suffix in resp. token_begin and end.
    Token& tok = parser_state.tokens().global_token();
    std::string prefix = tok.prefix_to_string();
    std::string suffix = tok.suffix_to_string();
    entry_point->inner_scope()->token_begin = {Token_t::ignore, prefix };
    entry_point->inner_scope()->token_end   = {Token_t::ignore, suffix };

    // Parse main code block
    parser_state.push_scope( entry_point->inner_scope() );
    Optional<Node*> block = parse_code_block( entry_point->inner_scope() );
    parser_state.pop_scope();

    if ( block )
    {
        parser_state.graph()->connect(entry_point->flow_out(),
                                      block->flow_in(),
                                      ConnectFlag_ALLOW_SIDE_EFFECTS);
    }
    else if ( parser_state.tokens().can_eat( ) )
    {
        parser_state.rollback();
        parser_state.graph()->on_reset.emit();
        LOG_WARNING("Parser", "Some token remains after getting an empty code block\n");
        LOG_MESSAGE("Parser", KO "Parse program.\n");
        return parser_state.graph()->root();
    }
    else
    {
        LOG_WARNING("Parser", "Program main block is empty\n");
    }

    parser_state.commit();
    parser_state.graph()->on_reset.emit();

    LOG_MESSAGE("Parser", OK "Parse program.\n");

    return parser_state.graph()->root();
}

tools::Optional<Node*> Nodlang::parse_scoped_block(Scope* scope)
{
    LOG_VERBOSE("Parser", "Parsing scoped block ...\n");

    auto scope_begin_token = parser_state.tokens().eat_if(Token_t::scope_begin);
    if ( !scope_begin_token )
    {
        LOG_VERBOSE("Parser", KO "Expecting main_scope begin token\n");
        return nullptr;
    }

    parser_state.start_transaction();

    // Handle nested scopes
    parser_state.push_scope( scope );
    Optional<Node*> first_atomic_block = parse_code_block( scope ); // no return check, allows empty scope
    parser_state.pop_scope();

    if ( Token scope_end_token = parser_state.tokens().eat_if(Token_t::scope_end) )
    {
        // Update scope's begin/end tokens
        scope->token_begin = scope_begin_token;
        scope->token_end   = scope_end_token;

        if ( !first_atomic_block )
        {
            Node* empty_instr = parser_state.graph()->create_empty_instruction();
            scope->push_back( empty_instr );
            first_atomic_block = empty_instr;
        }

        parser_state.commit();
        LOG_VERBOSE("Parser", OK "Scoped block parsed:\n%s\n", parser_state.tokens().to_string().c_str());
        return first_atomic_block;
    }
    else
    {
        LOG_VERBOSE("Parser", KO "Expecting close main_scope token\n");
    }

    scope->clear();
    parser_state.rollback();
    LOG_VERBOSE("Parser", KO "Scoped block parsed\n");
    return nullptr;
}

Optional<Node*> Nodlang::parse_code_block(Scope* scope)
{
    LOG_VERBOSE("Parser", "Parsing code block...\n" );

    //
    // Parse n atomic code blocks
    //
    parser_state.start_transaction();

    scope = scope ? scope : parser_state.current_scope();
    Node*  first_block       = nullptr;
    Node*  last_block        = scope ? scope->last_node() : nullptr;
    bool   block_end_reached = false;
    size_t block_size        = 0;

    while ( parser_state.tokens().can_eat() && !block_end_reached )
    {
        ASSERT( scope );
        if ( Node* curr_block = parse_atomic_code_block( scope ).data() )
        {
            // linked-list like
            if ( last_block )
            {
                if ( last_block->inner_scope() )
                {
                    for ( Node* last : last_block->inner_scope()->last_instr() )
                    {
                        parser_state.graph()->connect(last->flow_out(),
                                                      curr_block->flow_in(),
                                                      ConnectFlag_ALLOW_SIDE_EFFECTS );
                    }
                }
                else
                {
                    parser_state.graph()->connect(last_block->flow_out(),
                                                  curr_block->flow_in(),
                                                  ConnectFlag_ALLOW_SIDE_EFFECTS );
                }
            }

            if ( !first_block )
                first_block = curr_block;
            last_block = curr_block;
            block_size++;
        }
        else
            block_end_reached = true;
    }

    if ( first_block )
    {
        parser_state.commit();
        LOG_VERBOSE("Parser", OK "parse code block:\n%s\n", parser_state.tokens().to_string().c_str());
        return first_block;
    }

    parser_state.rollback();
    LOG_VERBOSE("Parser", KO "parse code block. Block size is %llu\n", block_size );
    return nullptr;
}

Optional<Slot*> Nodlang::parse_expression(u8_t _precedence, Optional<Slot*> _left_override)
{
    LOG_VERBOSE("Parser", "Parsing expression ...\n");

    /*
		Get the left-handed operand
	*/
    Optional<Slot*> left = _left_override;

    if (!parser_state.tokens().can_eat())
    {
        LOG_VERBOSE("Parser", OK "Last token reached\n");
        return left;
    }

    if ( !left ) left = parse_parenthesis_expression();
    if ( !left ) left = parse_unary_operator_expression(_precedence);
    if ( !left ) left = parse_function_call();
    if ( !left ) left = parse_variable_declaration(); // nullptr => variable won't be attached on the codeflow, it's a part of an expression..
    if ( !left ) left = parse_atomic_expression();

    if (!parser_state.tokens().can_eat())
    {
        LOG_VERBOSE("Parser", OK "Last token reached\n");
        return left;
    }

    if ( !left )
    {
        LOG_VERBOSE("Parser", OK "Left side is null, we return it\n");
        return left;
    }

    /*
		Get the right-handed operand
	*/
    Optional<Slot*> expression_out = parse_binary_operator_expression( _precedence, left.get() );
    if ( expression_out )
    {
        if (!parser_state.tokens().can_eat())
        {
            LOG_VERBOSE("Parser", OK "Right side parsed, and last token reached\n");
            return expression_out.data();
        }
        LOG_VERBOSE("Parser", OK "Right side parsed, continue with a recursive call...\n");
        return parse_expression(_precedence, expression_out);
    }

    LOG_VERBOSE("Parser", OK "Returning left side only\n");

    return left;
}

bool Nodlang::is_syntax_valid()
{
    // TODO: optimization: is this function really useful ? It check only few things.
    //                     The parsing steps that follow (parseProgram) is doing a better check, by looking to what exist in the Language.
    bool success = true;
    auto token = parser_state.tokens().begin();
    short int opened = 0;

    while (token != parser_state.tokens().end() && success)
    {
        switch (token->m_type)
        {
            case Token_t::parenthesis_open:
            {
                opened++;
                break;
            }
            case Token_t::parenthesis_close:
            {
                if (opened <= 0)
                {
                    LOG_ERROR("Parser",
                              "Syntax Error: Unexpected close bracket after \"... %s\" (position %llu)\n",
                              parser_state.tokens().range_to_string(token->m_index, -10).c_str(),
                              token->offset()
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
        LOG_ERROR("Parser", "Syntax Error: Bracket count mismatch, %i still opened.\n", opened);
        success = false;
    }

    return success;
}

bool Nodlang::tokenize(const std::string& _string)
{
    parser_state.reset_ribbon(const_cast<char *>(_string.data()), _string.length());
    return tokenize();
}

bool Nodlang::tokenize()
{
    LOG_MESSAGE("Parser", "Tokenization ...\n");

    size_t global_cursor       = 0;
    size_t ignored_chars_count = 0;

    while (global_cursor != parser_state.buffer_size() )
    {
        size_t current_cursor = global_cursor;
        Token  new_token = parse_token( parser_state.buffer(), parser_state.buffer_size(), global_cursor );

        if ( !new_token )
        {
            LOG_WARNING("Parser", KO "Unable to tokenize from \"%20s...\" (at index %llu)\n", parser_state.buffer_at(current_cursor), global_cursor);
            return false;
        }

        // accumulate ignored chars (see else case to know why)
        if( new_token.m_type == Token_t::ignore)
        {
            if (  parser_state.tokens().empty() )
            {
                parser_state.tokens().global_token().prefix_end_grow( new_token.length() );
                continue;
            }

            ignored_chars_count += new_token.length();
            continue;
        }

        if ( ignored_chars_count )
        {
            // case 1: if token type allows it => increase last token's prefix to wrap the ignored chars
            Token& back = parser_state.tokens().back();
            if ( accepts_suffix(back.m_type) )
            {
                back.suffix_end_grow(ignored_chars_count);
                LOG_VERBOSE("Parser", "      \"%s\" (update) \n", back.string().c_str() );
            }
            // case 2: increase prefix of the new_token up to wrap the ignored chars
            else if ( new_token )
            {
                new_token.prefix_begin_grow(ignored_chars_count);
            }
            ignored_chars_count = 0;
        }

        parser_state.tokens().push(new_token);
        LOG_VERBOSE("Parser", "%4llu) \"%s\" \n", new_token.m_index, new_token.string().c_str() );
    }

    // Append remaining ignored chars to the ribbon's suffix
    if ( ignored_chars_count )
    {
        LOG_VERBOSE("Parser", "Found ignored chars after tokenize, adding to the tokens suffix...\n");
        Token& tok = parser_state.tokens().global_token();
        tok.suffix_begin_grow( ignored_chars_count );
    }

    LOG_MESSAGE("Parser", OK "Tokenization.\n%s\n", parser_state.tokens().to_string().c_str() );

    return true;
}

Token Nodlang::parse_token(const char* buffer, size_t buffer_size, size_t& global_cursor) const
{
    const size_t                  start_pos  = global_cursor;
    const std::string::value_type first_char = buffer[start_pos];
    const size_t                  char_left  = buffer_size - start_pos;

    // comments
    if (first_char == '/' && char_left > 1)
    {
        size_t cursor      = start_pos + 1;
        char   second_char = buffer[cursor];
        if (second_char == '*' || second_char == '/')
        {
            // multi-line comment
            if (second_char == '*')
            {
                while (cursor != buffer_size && !(buffer[cursor] == '/' && buffer[cursor - 1] == '*'))
                {
                    ++cursor;
                }
            }
            // single-line comment
            else
            {
                while (cursor != buffer_size && buffer[cursor] != '\n' )
                {
                    ++cursor;
                }
            }

            ++cursor;
            global_cursor = cursor;
            return Token{Token_t::ignore, const_cast<char*>(buffer), start_pos, cursor - start_pos};
        }
    }

    // single-char
    auto single_char_found = m_token_t_by_single_char.find(first_char);
    if( single_char_found != m_token_t_by_single_char.end() )
    {
        ++global_cursor;
        const Token_t type = single_char_found->second;
        return Token{type, const_cast<char*>(buffer), start_pos, 1};
    }

    // operators
    switch (first_char)
    {
        case '=':
        {
            // "=>" or "=="
            auto cursor = start_pos + 1;
            auto second_char = buffer[cursor];
            if (cursor != buffer_size && (second_char == '>' || second_char == '=')) {
                ++cursor;
                global_cursor = cursor;
                return Token{Token_t::operator_, const_cast<char*>(buffer), start_pos, cursor - start_pos};
            }
            // "="
            global_cursor++;
            return Token{Token_t::operator_, const_cast<char*>(buffer), start_pos, 1};
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
            auto cursor = start_pos + 1;
            if (cursor != buffer_size && buffer[cursor] == '=') {
                ++cursor;
                // special case for "<=>" operator
                if (first_char == '<' && cursor != buffer_size && buffer[cursor] == '>') {
                    ++cursor;
                }
                global_cursor = cursor;
            } else {
                // <operator>
                global_cursor++;
            }
            return Token{Token_t::operator_, const_cast<char*>(buffer), start_pos, cursor - start_pos};
        }
    }

    // number (double)
    //     note: we accept zeros as prefix (ex: "0002.15454", or "01012")
    if ( std::isdigit(first_char) )
    {
        auto cursor = start_pos + 1;
        Token_t type = Token_t::literal_int;

        // integer
        while (cursor != buffer_size && std::isdigit(buffer[cursor]))
        {
            ++cursor;
        }

        // double
        if(cursor + 1 < buffer_size
           && buffer[cursor] == '.'      // has a decimal separator
            && std::isdigit(buffer[cursor + 1]) // followed by a digit
           )
        {
            auto local_cursor_decimal_separator = cursor;
            ++cursor;

            // decimal portion
            while (cursor != buffer_size && std::isdigit(buffer[cursor]))
            {
                ++cursor;
            }
            type = Token_t::literal_double;
        }
        global_cursor = cursor;
        return Token{type, const_cast<char*>(buffer), start_pos, cursor - start_pos};
    }

    // double-quoted string
    if (first_char == '"')
    {
        auto cursor = start_pos + 1;
        while (cursor != buffer_size && (buffer[cursor] != '"' || buffer[cursor - 1] == '\\'))
        {
            ++cursor;
        }
        ++cursor;
        global_cursor = cursor;
        return Token{Token_t::literal_string, const_cast<char*>(buffer), start_pos, cursor - start_pos};
    }

    // symbol (identifier or keyword)
    if ( std::isalpha(first_char) || first_char == '_' )
    {
        // parse symbol
        auto cursor = start_pos + 1;
        while (cursor != buffer_size && std::isalnum(buffer[cursor]) || buffer[cursor] == '_' )
        {
            ++cursor;
        }
        global_cursor = cursor;

        Token_t type = Token_t::identifier;

        auto hash = hash::hash( buffer + start_pos, cursor - start_pos );
        auto keyword_found = m_token_t_by_keyword.find( hash );
        if (keyword_found != m_token_t_by_keyword.end())
        {
            // a keyword has priority over identifier
            type = keyword_found->second;
        }
        return Token{type, const_cast<char*>(buffer), start_pos, cursor - start_pos};
    }
    return Token_t::none;
}

Optional<Slot*> Nodlang::parse_function_call()
{
    LOG_VERBOSE("Parser", "parse function call...\n");

    // Check if the minimum token count required is available ( 0: identifier, 1: open parenthesis, 2: close parenthesis)
    if (!parser_state.tokens().can_eat(3))
    {
        LOG_VERBOSE("Parser", KO "3 tokens min. are required\n");
        return nullptr;
    }

    parser_state.start_transaction();

    // Try to parse regular function: function(...)
    std::string fct_id;
    Token token_0 = parser_state.tokens().eat();
    Token token_1 = parser_state.tokens().eat();
    if (token_0.m_type == Token_t::identifier &&
        token_1.m_type == Token_t::parenthesis_open)
    {
        fct_id = token_0.word_to_string();
        LOG_VERBOSE("Parser", OK "Regular function pattern detected.\n");
    }
    else// Try to parse operator like (ex: operator==(..,..))
    {
        Token token_2 = parser_state.tokens().eat();// eat a "supposed open bracket>

        if (token_0.m_type == Token_t::keyword_operator && token_1.m_type == Token_t::operator_ && token_2.m_type == Token_t::parenthesis_open)
        {
            fct_id = token_1.word_to_string();// operator
            LOG_VERBOSE("Parser", OK "Operator function-like pattern detected.\n");
        }
        else
        {
            LOG_VERBOSE("Parser", KO "Not a function.\n");
            parser_state.rollback();
            return nullptr;
        }
    }
    std::vector<Slot*> result_slots;

    // Declare a new function prototype
    FunctionDescriptor* signature = FunctionDescriptor::create<any()>(fct_id.c_str());

    bool parsingError = false;
    while (!parsingError && parser_state.tokens().can_eat() &&
            parser_state.tokens().peek().m_type != Token_t::parenthesis_close)
    {
        Optional<Slot*> expression_out = parse_expression();
        if ( expression_out )
        {
            result_slots.push_back( expression_out.get() );
            signature->push_arg( expression_out->property->get_type() );
            parser_state.tokens().eat_if(Token_t::list_separator);
        }
        else
        {
            parsingError = true;
        }
    }

    // eat "close bracket supposed" token
    if ( !parser_state.tokens().eat_if(Token_t::parenthesis_close) )
    {
        LOG_WARNING("Parser", KO "Expecting parenthesis close\n");
        parser_state.rollback();
        return nullptr;
    }


    // Find the prototype in the language library
    FunctionNode* fct_node = parser_state.graph()->create_function(std::move(signature));

    for ( int i = 0; i < fct_node->get_arg_slots().size(); i++ )
    {
        // Connects each results to the corresponding input
        parser_state.graph()->connect_or_merge( result_slots.at(i), fct_node->get_arg_slot(i) );
    }

    parser_state.commit();
    LOG_VERBOSE("Parser", KO "Function call parsed:\n%s\n", parser_state.tokens().to_string().c_str() );

    return fct_node->value_out();
}

Optional<IfNode*> Nodlang::parse_if_block()
{
    parser_state.start_transaction();

    Token if_token = parser_state.tokens().eat_if(Token_t::keyword_if);
    if ( !if_token )
    {
        return {};
    }

    LOG_VERBOSE("Parser", "Parsing conditional structure...\n");

    bool              result = false;
    Optional<Slot*>   block_end;
    Optional<IfNode*> if_node;
    Optional<IfNode*> else_if;

    if_node = parser_state.graph()->create_cond_struct();
    Scope* if_scope = if_node->inner_scope();
    parser_state.push_scope(if_scope);
    if_node->token_if  = parser_state.tokens().get_eaten();
    Scope* false_scope = if_scope->child_scope_at(Branch_FALSE);
    Scope* true_scope  = if_scope->child_scope_at(Branch_TRUE);

    if (parser_state.tokens().eat_if(Token_t::parenthesis_open) )
    {
        LOG_VERBOSE("Parser", "Parsing conditional structure's condition...\n");

        // condition
        parse_expression_block(if_node->condition_in());

        if (parser_state.tokens().eat_if(Token_t::parenthesis_close) )
        {
            // scope
            if ( Optional<Node*> block = parse_atomic_code_block( true_scope ) )
            {
                parser_state.graph()->connect(
                        if_node->branch_out(Branch_TRUE),
                        block->flow_in(),
                        ConnectFlag_ALLOW_SIDE_EFFECTS);

                // else
                if ( parser_state.tokens().eat_if(Token_t::keyword_else) )
                {
                    if_node->token_else = parser_state.tokens().get_eaten();

                    if ( else_if = parse_if_block() )
                    {
                        parser_state.graph()->connect(
                                if_node->branch_out(Branch_FALSE),
                                else_if->flow_in(),
                                ConnectFlag_ALLOW_SIDE_EFFECTS);
                        result = true;
                        LOG_VERBOSE("Parser", OK "else if block parsed.\n");
                    }
                    else if ( Optional<Node*> else_block = parse_atomic_code_block( false_scope ))
                    {

                        parser_state.graph()->connect(
                                if_node->branch_out(Branch_FALSE),
                                else_block->flow_in(),
                                ConnectFlag_ALLOW_SIDE_EFFECTS);
                        result = true;
                        LOG_VERBOSE("Parser", OK "else block parsed.\n");
                    }
                    else
                    {
                        LOG_VERBOSE("Parser", KO "Single instruction or main_scope expected\n");
                    }
                }
                else
                {
                    false_scope->token_begin = {Token_t::ignore};
                    false_scope->token_end   = {Token_t::ignore};
                    result = true;
                }
            }
            else
            {
                LOG_VERBOSE("Parser", KO "Single instruction or main_scope expected\n");
            }
        }
        else
        {
            LOG_VERBOSE("Parser", KO "Close bracket expected\n");
        }
    }
    parser_state.pop_scope();

    if ( result )
    {
        parser_state.commit();
        LOG_VERBOSE("Parser", OK "Parse conditional structure:\n%s\n", parser_state.tokens().to_string().c_str() );
        return if_node;
    }

    parser_state.graph()->destroy( if_node.data() );
    parser_state.graph()->destroy(else_if.data() );
    parser_state.rollback();
    LOG_VERBOSE("Parser", KO "Parse conditional structure \n");

    return nullptr;
}

Optional<ForLoopNode*> Nodlang::parse_for_block()
{
    bool                   success = false;
    Optional<ForLoopNode*> for_node;
    Optional<Slot*>        true_branch_end;

    parser_state.start_transaction();

    if ( Token token_for = parser_state.tokens().eat_if(Token_t::keyword_for) )
    {

        LOG_VERBOSE("Parser", "Parsing for loop ...\n");

        for_node = parser_state.graph()->create_for_loop();
        for_node->token_for = token_for;

        Token open_bracket = parser_state.tokens().eat_if(Token_t::parenthesis_open);
        if ( open_bracket)
        {
            LOG_VERBOSE("Parser", "Parsing for init/condition/iter instructions ...\n");

            parser_state.push_scope( for_node->inner_scope() );

            // first we parse three instructions, no matter if we find them, we'll continue (we are parsing something abstract)

            // parse init; condition; iteration or nothing
            parse_expression_block(for_node->initialization_slot())
            && parse_expression_block(for_node->condition_in())
            && parse_expression_block(for_node->iteration_slot());

            // parse parenthesis close
            if ( Token parenthesis_close = parser_state.tokens().eat_if(Token_t::parenthesis_close) )
            {
                Scope* true_scope = for_node->inner_scope()->child_scope_at(Branch_TRUE);
                Optional<Node*> block = parse_atomic_code_block( true_scope );
                if ( block )
                {
                    parser_state.graph()->connect(for_node->branch_out(Branch_TRUE),
                                                  block->flow_in(),
                                                  ConnectFlag_ALLOW_SIDE_EFFECTS);
                    success = true;
                    LOG_VERBOSE("Parser", "Scope or single instruction found\n");
                }
                else
                {
                    LOG_VERBOSE("Parser", KO "Scope or single instruction expected\n");
                }
            }
            else
            {
                LOG_VERBOSE("Parser", KO "Close parenthesis was expected.\n");
            }
            parser_state.pop_scope();
        }
        else
        {
            LOG_VERBOSE("Parser", KO "Open parenthesis was expected.\n");
        }
    }

    if ( success )
    {
        LOG_VERBOSE("Parser", KO "For block parsed\n");
        parser_state.commit();
        return for_node;
    }

    parser_state.rollback();
    if ( true_branch_end )
        parser_state.graph()->destroy( true_branch_end->node );
    parser_state.graph()->destroy(for_node.data() );
    LOG_VERBOSE("Parser", KO "Could not parse for block\n");
    return nullptr;
}

Optional<WhileLoopNode*> Nodlang::parse_while_block()
{
    bool                     success = false;
    Optional<WhileLoopNode*> while_node;
    Optional<Node*>          code_block;

    parser_state.start_transaction();

    if ( Token token_while = parser_state.tokens().eat_if(Token_t::keyword_while) )
    {
        LOG_VERBOSE("Parser", "Parsing while ...\n");

        while_node = parser_state.graph()->create_while_loop();
        while_node->token_while = token_while;

        parser_state.push_scope( while_node->inner_scope() );

        if ( Token open_bracket = parser_state.tokens().eat_if(Token_t::parenthesis_open) )
        {
            LOG_VERBOSE("Parser", "Parsing while condition ... \n");

            // Parse an optional condition
            parse_expression_block(while_node->condition_in());

            if (parser_state.tokens().eat_if(Token_t::parenthesis_close) )
            {
                Scope* true_scope = while_node->inner_scope()->child_scope_at(Branch_TRUE);
                code_block = parse_atomic_code_block( true_scope );
                if ( code_block )
                {
                    parser_state.graph()->connect(
                            while_node->branch_out(Branch_TRUE),
                            code_block->flow_in(),
                            ConnectFlag_ALLOW_SIDE_EFFECTS);
                    success = true;
                }
                else
                {
                    LOG_VERBOSE("Parser", KO "Scope or single instruction expected\n");
                }
            }
            else
            {
                LOG_VERBOSE("Parser", KO "Parenthesis close expected\n");
            }
        }
        else
        {
            LOG_VERBOSE("Parser", KO "Parenthesis close expected\n");
        }
        parser_state.pop_scope();
    }

    if ( success )
    {
        LOG_VERBOSE("Parser", "Parsing while:\n%s\n", parser_state.tokens().to_string().c_str() );
        parser_state.commit();
        return while_node;
    }

    parser_state.rollback();
    parser_state.graph()->destroy(while_node.data() );

    return nullptr;
}

Optional<Slot*> Nodlang::parse_variable_declaration()
{
    if (!parser_state.tokens().can_eat(2))
    {
        return nullptr;
    }

    parser_state.start_transaction();

    bool  success          = false;
    Token type_token       = parser_state.tokens().eat();
    Token identifier_token = parser_state.tokens().eat();

    if (type_token.is_keyword_type() && identifier_token.m_type == Token_t::identifier)
    {
        const TypeDescriptor* type = get_type(type_token.m_type);
        VariableNode* variable_node = parser_state.graph()->create_variable(type, identifier_token.word_to_string() );
        variable_node->set_flags(VariableFlag_DECLARED);
        variable_node->set_type_token( type_token );
        variable_node->set_identifier_token( identifier_token );

        // declaration with assignment ?
        Token operator_token = parser_state.tokens().eat_if(Token_t::operator_);
        if (operator_token && operator_token.word_len() == 1 && *operator_token.word() == '=')
        {
            // an expression is expected
            if ( Optional<Slot*> expression_out = parse_expression() )
            {
                // expression's out ----> variable's in
                parser_state.graph()->connect_to_variable( expression_out.get(), variable_node );

                variable_node->set_operator_token( operator_token );
                success = true;
            }
            else
            {
                LOG_VERBOSE("Parser", KO "Initialization expression expected for %s\n", identifier_token.word_to_string().c_str());
            }
        }
            // Declaration without assignment
        else
        {
            success = true;
        }

        if ( success )
        {
            LOG_VERBOSE("Parser", OK "Variable declaration: %s %s\n",
                        variable_node->value()->get_type()->get_name(),
                        identifier_token.word_to_string().c_str());
            parser_state.commit();
            return variable_node->value_out();
        }

        LOG_VERBOSE("Parser", KO "Initialization expression expected for %s\n", identifier_token.word_to_string().c_str());
        parser_state.graph()->destroy( variable_node );
    }

    parser_state.rollback();
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------------
// [SECTION] C. Serializer --------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------

const Slot* Nodlang::serialize_invokable(std::string &_out, const FunctionNode* _node) const
{
    if ( _node->type() == NodeType_OPERATOR )
    {
        const std::vector<Slot*>& args = _node->get_arg_slots();
        int precedence = get_precedence(_node->get_func_type());

        const FunctionDescriptor* func_type  = _node->get_func_type();
        switch (func_type->get_arg_count())
        {
            case 2:
            {
                // Left part of the expression
                {
                    const FunctionDescriptor* l_func_type = _node->get_connected_function_type(LEFT_VALUE_PROPERTY);
                    bool needs_braces = l_func_type && get_precedence(l_func_type) < precedence;
                    SerializeFlags flags = SerializeFlag_RECURSE
                                         | needs_braces * SerializeFlag_WRAP_WITH_BRACES ;
                    serialize_input( _out, args[0], flags );
                }

                // Operator
                VERIFY( _node->get_identifier_token(), "identifier token should have been assigned in parse_function_call");
                serialize_token( _out, _node->get_identifier_token() );

                // Right part of the expression
                {
                    const FunctionDescriptor* r_func_type = _node->get_connected_function_type(RIGHT_VALUE_PROPERTY);
                    bool needs_braces = r_func_type && get_precedence(r_func_type) < precedence;
                    SerializeFlags flags = SerializeFlag_RECURSE
                                         | needs_braces * SerializeFlag_WRAP_WITH_BRACES ;
                    serialize_input( _out, args[1], flags );
                }
                break;
            }

            case 1:
            {
                // operator ( ... innerOperator ... )   ex:   -(a+b)

                ASSERT( _node->get_identifier_token() );
                serialize_token(_out, _node->get_identifier_token());

                bool needs_braces    = _node->get_connected_function_type(LEFT_VALUE_PROPERTY) != nullptr;
                SerializeFlags flags = SerializeFlag_RECURSE
                                     | needs_braces * SerializeFlag_WRAP_WITH_BRACES;
                serialize_input( _out, args[0], flags );
                break;
            }
        }
    }
    else
    {
        serialize_func_call(_out, _node->get_func_type(), _node->get_arg_slots());
    }

    return _node->value_out();
}

std::string &Nodlang::serialize_func_call(std::string &_out, const FunctionDescriptor *_signature, const std::vector<Slot*> &inputs) const
{
    _out.append( _signature->get_identifier() );
    serialize_token_t(_out, Token_t::parenthesis_open);

    for (const Slot* input_slot : inputs)
    {
        ASSERT( input_slot->has_flags(SlotFlag_INPUT) );
        if ( input_slot != inputs.front())
        {
            serialize_token_t(_out, Token_t::list_separator);
        }
        serialize_input( _out, input_slot, SerializeFlag_RECURSE );
    }

    serialize_token_t(_out, Token_t::parenthesis_close);
    return _out;
}

std::string &Nodlang::serialize_invokable_sig(std::string &_out, const IInvokable* _invokable) const
{
    return serialize_func_sig(_out, _invokable->get_sig());
}

std::string &Nodlang::serialize_func_sig(std::string &_out, const FunctionDescriptor *_signature) const
{
    serialize_type(_out, _signature->get_return_type());
    _out.append(" ");
    _out.append(_signature->get_identifier());
    serialize_token_t(_out, Token_t::parenthesis_open);

    auto args = _signature->get_args();
    for (auto it = args.begin(); it != args.end(); it++)
    {
        if (it != args.begin())
        {
            serialize_token_t(_out, Token_t::list_separator);
            _out.append(" ");
        }
        serialize_type(_out, it->type);
    }

    serialize_token_t(_out, Token_t::parenthesis_close);
    return _out;
}

std::string &Nodlang::serialize_type(std::string &_out, const TypeDescriptor *_type) const
{
    auto found = m_keyword_by_type_id.find(_type->id());
    if (found != m_keyword_by_type_id.cend())
    {
        return _out.append(found->second);
    }
    return _out;
}

std::string& Nodlang::serialize_variable_ref(std::string &_out, const VariableRefNode* _node) const
{
    return serialize_token( _out, _node->get_identifier_token() );
}

std::string& Nodlang::serialize_variable(std::string &_out, const VariableNode *_node) const
{
    // 1. Serialize variable's type

    // If parsed
    if ( _node->get_type_token() )
    {
        serialize_token(_out, _node->get_type_token());
    }
    else // If created in the graph by the user
    {
        serialize_type(_out, _node->value()->get_type());
        _out.append(" ");
    }

    // 2. Serialize variable identifier
    serialize_token( _out, _node->get_identifier_token() );

    // 3. Initialisation
    //    When a VariableNode has its input connected, we serialize it as its initialisation expression

    const Slot* slot = _node->value_in();
    if ( slot->adjacent_count() != 0 )
    {
        if ( _node->get_operator_token() )
            _out.append(_node->get_operator_token().string());
        else
            _out.append(" = ");

        serialize_input( _out, slot, SerializeFlag_RECURSE );
    }
    return _out;
}

std::string &Nodlang::serialize_input(std::string& _out, const Slot* slot, SerializeFlags _flags ) const
{
    ASSERT( slot->has_flags( SlotFlag_INPUT ) );

    const Slot*     adjacent_slot     = slot->first_adjacent();
    const Property* adjacent_property = adjacent_slot != nullptr ? adjacent_slot->property
                                                                 : nullptr;
    // Append open brace?
    if ( _flags & SerializeFlag_WRAP_WITH_BRACES )
        serialize_token_t(_out, Token_t::parenthesis_open);

    if ( adjacent_property == nullptr )
    {
        // Simply serialize this property
        serialize_property(_out, slot->property);
    }
    else
    {
        VERIFY( _flags & SerializeFlag_RECURSE, "Why would you call serialize_input without RECURSE flag?");
        // Append token prefix?
        if (const Token& adjacent_token = adjacent_property->token())
            if ( adjacent_token )
                _out.append(adjacent_token.prefix(), adjacent_token.prefix_len() );

        // Serialize adjacent slot
        serialize_value_out(_out, adjacent_slot, SerializeFlag_RECURSE);

        // Append token suffix?
        if (const Token& adjacent_token = adjacent_property->token())
            if ( adjacent_token )
                _out.append(adjacent_token.suffix(), adjacent_token.suffix_len() );
    }

    // Append close brace?
    if ( _flags & SerializeFlag_WRAP_WITH_BRACES )
        serialize_token_t(_out, Token_t::parenthesis_close);

    return _out;
}

std::string &Nodlang::serialize_value_out(std::string& _out, const Slot* slot, SerializeFlags _flags) const
{
    // If output is node's output value, we serialize the node
    if( slot == slot->node->value_out() )
    {
        serialize_node(_out, slot->node, _flags);
        return _out;
    }

    // Otherwise, it might be a variable reference, so we serialize the identifier only
    ASSERT( slot->node->type() == NodeType_VARIABLE ); // Can't be another type
    auto variable = static_cast<const VariableNode*>( slot->node );
    VERIFY( slot == variable->ref_out(), "Cannot serialize an other slot from a VariableNode");
    return _out.append( variable->get_identifier() );
}

std::string& Nodlang::serialize_node(std::string &_out, const Node* node, SerializeFlags _flags ) const
{
    ASSERT( _flags == SerializeFlag_RECURSE ); // The only flag configuration handled for now

    switch ( node->type() )
    {
        case NodeType_BLOCK_IF:
            serialize_cond_struct(_out, static_cast<const IfNode*>(node) );
            break;
        case NodeType_BLOCK_FOR_LOOP:
            serialize_for_loop(_out, static_cast<const ForLoopNode*>(node) );
            break;
        case NodeType_BLOCK_WHILE_LOOP:
            serialize_while_loop(_out, static_cast<const WhileLoopNode*>(node) );
            break;
        case NodeType_LITERAL:
            serialize_literal(_out, static_cast<const LiteralNode*>(node) );
            break;
        case NodeType_VARIABLE:
            serialize_variable(_out, static_cast<const VariableNode*>(node));
            break;
        case NodeType_VARIABLE_REF:
            serialize_variable_ref(_out, static_cast<const VariableRefNode*>(node));
            break;
        case NodeType_FUNCTION:
            [[fallthrough]];
        case NodeType_OPERATOR:
            serialize_invokable(_out, static_cast<const FunctionNode*>(node) );
            break;
        case NodeType_EMPTY_INSTRUCTION:
            serialize_empty_instruction(_out, node);
            break;
        case NodeType_ENTRY_POINT:
            serialize_scope(_out, node->inner_scope() );
            break;
        default:
            VERIFY(false, "Unhandled NodeType, can't serialize");
    }
    serialize_token(_out, node->suffix() );

    return _out;
}

std::string& Nodlang::serialize_scope(std::string &_out, const Scope* scope) const
{
    serialize_token(_out, scope->token_begin);
    for(Node* node : scope->child_node() )
    {
        serialize_node(_out, node, SerializeFlag_RECURSE);
    }
    serialize_token(_out, scope->token_end);

    return _out;
}

std::string &Nodlang::serialize_token(std::string& _out, const Token& _token) const
{
    // Skip a null token
    if ( !_token )
        return _out;

    // optimized case, if we have a "word", we can serialize the whole token
    if ( _token.word_len() != 0 )
        return _out.append(_token.begin(), _token.length());

    // append prefix, default word, and suffix
    if (_token.prefix() )
        _out.append(_token.prefix(), _token.prefix_len());
    serialize_token_t( _out, _token.m_type ); // <---------- default word!
    if (_token.suffix() )
        _out.append(_token.suffix(), _token.suffix_len());

    return _out;
}

std::string& Nodlang::serialize_graph(std::string &_out, const Graph* graph ) const
{
    if ( const Node* root = graph->root().get() )
        serialize_node(_out, root, SerializeFlag_RECURSE);
    else
        LOG_ERROR("Serializer", "a root child_node is expected to serialize the graph\n");
    return _out;
}

std::string& Nodlang::serialize_bool(std::string& _out, bool b) const
{
    return _out.append( b ? "true" : "false");
}

std::string& Nodlang::serialize_int(std::string& _out, int i) const
{
    return _out.append( std::to_string(i) );
}

std::string& Nodlang::serialize_double(std::string& _out, double d) const
{
    return _out.append( format::number(d) );
}

std::string& Nodlang::serialize_for_loop(std::string &_out, const ForLoopNode *_for_loop) const
{
    serialize_token(_out, _for_loop->token_for);
    serialize_token_t(_out, Token_t::parenthesis_open);
    {
        const Slot* init_slot = _for_loop->find_slot_by_property_name( INITIALIZATION_PROPERTY, SlotFlag_INPUT );
        const Slot* cond_slot = _for_loop->find_slot_by_property_name( CONDITION_PROPERTY, SlotFlag_INPUT );
        const Slot* iter_slot = _for_loop->find_slot_by_property_name( ITERATION_PROPERTY, SlotFlag_INPUT );
        serialize_input( _out, init_slot, SerializeFlag_RECURSE );
        serialize_input( _out, cond_slot, SerializeFlag_RECURSE );
        serialize_input( _out, iter_slot, SerializeFlag_RECURSE );
    }
    serialize_token_t(_out, Token_t::parenthesis_close);

    const std::vector<Scope *>& scopes = _for_loop->inner_scope()->child_scope();
    serialize_scope(_out, scopes[Branch_TRUE]);

    return _out;
}

std::string& Nodlang::serialize_while_loop(std::string &_out, const WhileLoopNode *_while_loop_node) const
{
    // while
    serialize_token(_out, _while_loop_node->token_while);

    // condition
    SerializeFlags flags = SerializeFlag_RECURSE
                         | SerializeFlag_WRAP_WITH_BRACES;
    serialize_input(_out, _while_loop_node->condition_in(), flags );

    if ( const Scope* branch = _while_loop_node->inner_scope()->child_scope().at(Branch_TRUE) )
    {
        serialize_scope(_out, branch);
    }

    return _out;
}


std::string& Nodlang::serialize_cond_struct(std::string &_out, const IfNode* _condition_struct ) const
{
    // if
    serialize_token(_out, _condition_struct->token_if);

    // condition
    SerializeFlags flags = SerializeFlag_RECURSE
                         | SerializeFlag_WRAP_WITH_BRACES;
    serialize_input(_out, _condition_struct->condition_in(), flags );

    // scope when condition is true
    const std::vector<Scope *>& if_scope = _condition_struct->inner_scope()->child_scope();
    serialize_scope(_out, if_scope[Branch_TRUE] );

    // when condition is false
    serialize_token(_out, _condition_struct->token_else);
    serialize_scope(_out, if_scope[Branch_FALSE] );

    return _out;
}

// Language definition ------------------------------------------------------------------------------------------------------------

const IInvokable* Nodlang::find_function(const char* _signature_hint) const
{
    if (_signature_hint == nullptr)
    {
        return nullptr;
    }

    auto hash = hash::hash_cstr(_signature_hint);
    return find_function( hash );
}

const tools::IInvokable* Nodlang::find_function(u32_t _hash) const
{
    auto found = m_functions_by_signature.find(_hash);
    if ( found != m_functions_by_signature.end())
    {
        return found->second;
    }
    return nullptr;
}

const tools::IInvokable* Nodlang::find_function(const FunctionDescriptor* _type) const
{
    if (!_type)
    {
        return nullptr;
    }
    auto exact = find_function_exact(_type);
    if (!exact) return find_function_fallback(_type);
    return exact;
}

std::string& Nodlang::serialize_property(std::string& _out, const Property* _property) const
{
    return serialize_token(_out, _property->token());
}

const tools::IInvokable* Nodlang::find_function_exact(const FunctionDescriptor* _other_type) const
{
    for(auto* invokable : m_functions)
        if ( invokable->get_sig()->is_exactly(_other_type) )
            return invokable;
    return nullptr;
}

const tools::IInvokable* Nodlang::find_function_fallback(const FunctionDescriptor* _other_type) const
{
    for(auto* invokable : m_functions)
        if ( invokable->get_sig()->is_compatible(_other_type) )
            return invokable;
    return nullptr;
}

const tools::IInvokable* Nodlang::find_operator_fct_exact(const FunctionDescriptor* _other_type) const
{
    if (!_other_type)
        return nullptr;

    for(auto* invokable : m_operators_impl)
        if ( invokable->get_sig()->is_exactly(_other_type) )
            return invokable;

    return nullptr;
}

const tools::IInvokable* Nodlang::find_operator_fct(const FunctionDescriptor *_type) const
{
    if (!_type)
    {
        return nullptr;
    }
    const tools::IInvokable* invokable = find_operator_fct_exact(_type);
    if (invokable != nullptr)
        return invokable;
    return find_operator_fct_fallback(_type);
}

const tools::IInvokable* Nodlang::find_operator_fct_fallback(const FunctionDescriptor* _other_type) const
{
    if (!_other_type)
        return nullptr;

    for(auto* invokable : m_operators_impl)
        if ( invokable->get_sig()->is_compatible(_other_type) )
            return invokable;

    return nullptr;
}

void Nodlang::add_function(const tools::IInvokable* _invokable)
{
    m_functions.push_back(_invokable);

    std::string type_as_string;
    serialize_func_sig(type_as_string, _invokable->get_sig());

    // Stops if no operator having the same identifier and argument count is found
    if (!find_operator(_invokable->get_sig()->get_identifier(), static_cast<Operator_t>(_invokable->get_sig()->get_arg_count())))
    {
        LOG_VERBOSE("Nodlang", "add function: %s (in m_functions)\n", type_as_string.c_str());
        return;
    }

    // Register the invokable as an operator implementation
    m_operators_impl.push_back(_invokable);
    LOG_VERBOSE("Nodlang", "add operator: %s (in m_functions and m_operator_implems)\n", type_as_string.c_str());
}

const Operator *Nodlang::find_operator(const std::string &_identifier, Operator_t operator_type) const
{
    auto is_exactly = [&](const Operator *op) {
        return op->identifier == _identifier && op->type == operator_type;
    };

    auto found = std::find_if(m_operators.cbegin(), m_operators.cend(), is_exactly);

    if (found != m_operators.end())
        return *found;

    return nullptr;
}

std::string& Nodlang::serialize_token_t(std::string& _out, Token_t _token_t) const
{
    switch (_token_t)
    {
        case Token_t::end_of_line:     return _out.append("\n"); // TODO: handle all platforms
        case Token_t::operator_:       return _out.append("operator");
        case Token_t::identifier:      return _out.append("identifier");
        case Token_t::literal_string:  return _out.append("\"\"");
        case Token_t::literal_double:  [[fallthrough]];
        case Token_t::literal_int:     return _out.append("0");
        case Token_t::literal_bool:    return _out.append("false");
        case Token_t::literal_any:     [[fallthrough]];
        case Token_t::ignore:          [[fallthrough]];
        case Token_t::literal_unknown: return _out;
        default:
        {
            {
                auto found = m_keyword_by_token_t.find(_token_t);
                if (found != m_keyword_by_token_t.cend())
                {
                    return _out.append(found->second);
                }
            }
            {
                auto found = m_single_char_by_keyword.find(_token_t);
                if (found != m_single_char_by_keyword.cend())
                {
                    _out.push_back(found->second);
                    return _out;
                }
            }
            return _out.append("<?>");
        }
    }
}

std::string Nodlang::serialize_type(const TypeDescriptor *_type) const
{
    std::string result;
    serialize_type(result, _type);
    return result;
}

std::string Nodlang::serialize_token_t(Token_t _token) const
{
    std::string result;
    serialize_token_t(result, _token);
    return result;
}

int Nodlang::get_precedence( const tools::FunctionDescriptor* _func_type) const
{
    if (!_func_type)
        return std::numeric_limits<int>::min(); // default

    const Operator* operator_ptr = find_operator(_func_type->get_identifier(), static_cast<Operator_t>(_func_type->get_arg_count()));

    if (operator_ptr)
        return operator_ptr->precedence;
    return std::numeric_limits<int>::max();
}

const TypeDescriptor* Nodlang::get_type(Token_t _token) const
{
    auto found = m_type_by_token_t.find(_token);
    if ( found != m_type_by_token_t.end() )
        return found->second;
    return nullptr;
}

Token Nodlang::parse_token(const std::string &_string) const
{
    size_t cursor = 0;
    return parse_token( const_cast<char*>(_string.data()), _string.length(), cursor);
}

bool Nodlang::accepts_suffix(Token_t type) const
{
    return    type != Token_t::identifier          // identifiers must stay clean because they are reused
              && type != Token_t::parenthesis_open    // ")" are lost when creating AST
              && type != Token_t::parenthesis_close;  // "(" are lost when creating AST
}

Token_t Nodlang::to_literal_token(const TypeDescriptor *type) const
{
    if (type == type::get<double>() )
        return Token_t::literal_double;
    if (type == type::get<i16_t>() )
        return Token_t::literal_int;
    if (type == type::get<int>() )
        return Token_t::literal_int;
    if (type == type::get<bool>() )
        return Token_t::literal_bool;
    if (type == type::get<std::string>() )
        return Token_t::literal_string;
    if (type == type::get<any>() )
        return Token_t::literal_any;
    return Token_t::literal_unknown;
}

Optional<Node*> Nodlang::parse_atomic_code_block(Scope* scope)
{
    LOG_VERBOSE("Parser", "Parsing atomic code block ..\n");

    Optional<Node*> block;

         if (block = parse_expression_block() );
    else if (block = parse_if_block() );
    else if (block = parse_for_block() );
    else if (block = parse_while_block() );
    else     block = parse_scoped_block(scope);

    // empty atomic block?
    if ( !block )
    {
        if (Token tok = parser_state.tokens().eat_if(Token_t::end_of_instruction))
        {
            Node *empty_instr = parser_state.graph()->create_empty_instruction();
            empty_instr->value()->set_token(tok);
            block = empty_instr;
        }
    }

    if ( block.valid() )
    {
        if ( Token end_of_instr_tok = parser_state.tokens().eat_if(Token_t::end_of_instruction) )
        {
            block->set_suffix( end_of_instr_tok );
        }

        LOG_VERBOSE("Parser", OK "Block found (class %s)\n", block->get_class()->get_name() );
        return block;
    }

    LOG_VERBOSE("Parser", KO "No block found\n");
    return nullptr;
}

std::string& Nodlang::serialize_literal(std::string &_out, const LiteralNode* node) const
{
    return serialize_property( _out, node->value() );
}

std::string& Nodlang::serialize_empty_instruction(std::string &_out, const Node* node) const
{
    return serialize_token(_out, node->value()->token() );
}

void Nodlang::ParserState::reset_graph(Graph* new_graph)
{
    new_graph->clear();
    _graph = new_graph; // memory not owned
}

void Nodlang::ParserState::reset_scope_stack()
{
    while(!_scope.empty())
        _scope.pop();
}

void Nodlang::ParserState::reset_ribbon(const char* new_buf, size_t new_size)
{
    ASSERT( new_size == 0 || new_buf != nullptr);
    _buffer = { new_buf, new_size };
    _ribbon.reset( new_buf, new_size );
}

Nodlang* ndbl::init_language()
{
    ASSERT(g_language == nullptr);
    g_language = new Nodlang();
    return g_language;
}

Nodlang* ndbl::get_language()
{
    return g_language;
}

void ndbl::shutdown_language(Nodlang* _language)
{
    ASSERT(g_language == _language); // singleton for now
    ASSERT(g_language != nullptr);
    delete g_language;
    g_language = nullptr;
}

