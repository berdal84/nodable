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
#include <string>
#include <chrono>
#include <cctype> // isdigit, isalpha, and isalnum.

#include "tools/core/reflection/reflection"
#include "tools/core/format.h"
#include "tools/core/log.h"
#include "tools/core/hash.h"

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
    , _state()
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
    _state.reset_scope_stack();
    _state.reset_graph(graph_out );

    LOG_VERBOSE("Parser", "Parsing ...\n%s\n", code.c_str());

    if ( !tokenize(code) )
    {
        return false;
    }

    if (!is_syntax_valid())
    {
        return false;
    }

    Nodlang::FlowPath path = parse_program();

    if ( path.out.empty() )
        return false;

    if (_state.tokens().can_eat() )
    {
        _state.graph()->clear();
        LOG_VERBOSE("Parser", KO "End of token ribbon expected\n");
        LOG_VERBOSE("Parser", "%s", format::title("TokenRibbon").c_str());
        for (const Token& each_token : _state.tokens() )
        {
            LOG_VERBOSE("Parser", "token idx %i: %s\n", each_token.m_index, each_token.json().c_str());
        }
        LOG_VERBOSE("Parser", "%s", format::title("TokenRibbon end").c_str());
        auto curr_token = _state.tokens().peek();
        LOG_VERBOSE("Parser", KO "Failed to parse from token %llu/%llu and above.\n", curr_token.m_index, _state.tokens().size());
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

Optional<Slot*> Nodlang::token_to_slot(const Token& _token)
{
    if (_token.m_type == Token_t::identifier)
    {
        std::string identifier = _token.word_to_string();
        ASSERT(_state.current_scope());
        if( VariableNode* existing_variable = _state.current_scope()->find_variable_recursively(identifier) )
        {
            return existing_variable->ref_out();
        }

        if ( !m_strict_mode )
        {
            // Insert a VariableNodeRef with "any" type
            LOG_WARNING( "Parser", "%s is not declared (strict mode), abstract graph can be generated but compilation will fail.\n",
                         _token.word_to_string().c_str() );
            VariableRefNode* ref = _state.graph()->create_variable_ref();
            ref->value()->set_token(_token );
            return ref->value_out();
        }

        LOG_ERROR( "Parser", "%s is not declared (strict mode) \n", _token.word_to_string().c_str() );
        return nullptr;
    }

    LiteralNode* literal{};

    switch (_token.m_type)
    {
        case Token_t::literal_bool:   literal = _state.graph()->create_literal<bool>();        break;
        case Token_t::literal_int:    literal = _state.graph()->create_literal<i32_t>();       break;
        case Token_t::literal_double: literal = _state.graph()->create_literal<double>();      break;
        case Token_t::literal_string: literal = _state.graph()->create_literal<std::string>(); break;
        default:
            break; // we don't want to throw
    }

    if ( literal )
    {
        LOG_VERBOSE("Parser", OK "Token %s converted to a Literal %s\n",
                    _token.word_to_string().c_str(),
                    literal->value()->get_type()->name());
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

    if (!_state.tokens().can_eat(2))
    {
        LOG_VERBOSE("Parser", KO "Not enough tokens\n");
        return nullptr;
    }

    _state.start_transaction();
    const Token operator_token = _state.tokens().eat();
    const Token operand_token  = _state.tokens().peek();

    // Structure check
    const bool isValid = operator_token.m_type == Token_t::operator_ &&
                         operand_token.m_type != Token_t::operator_;

    if (!isValid)
    {
        _state.rollback();
        LOG_VERBOSE("Parser", KO "Unexpected tokens\n");
        return nullptr;
    }

    std::string word = operator_token.word_to_string();  // FIXME: avoid std::string copy, use hash
    const Operator *ope = find_operator(word, Operator_t::Binary);
    if (ope == nullptr)
    {
        LOG_VERBOSE("Parser", KO "Operator %s not found\n", word.c_str());
        _state.rollback();
        return nullptr;
    }

    // Precedence check
    if (ope->precedence <= _precedence && _precedence > 0)
    {// always update the first operation if they have the same precedence or less.
        LOG_VERBOSE("Parser", KO "Has lower precedence\n");
        _state.rollback();
        return nullptr;
    }

    // Parse right expression
    if ( Optional<Slot*> right = parse_expression(ope->precedence) )
    {
        // Create a function signature according to ltype, rtype and operator word
        FunctionDescriptor type;
        type.init<any(any, any)>(ope->identifier.c_str());
        type.arg_at(0).type = _left->property->get_type();
        type.arg_at(1).type = right->property->get_type();

        FunctionNode* binary_op = _state.graph()->create_operator( type );
        binary_op->set_identifier_token( operator_token );
        binary_op->lvalue_in()->property->token().m_type = _left->property->token().m_type;
        binary_op->rvalue_in()->property->token().m_type = right->property->token().m_type;

        _state.graph()->connect_or_merge(_left         , binary_op->lvalue_in());
        _state.graph()->connect_or_merge(right.get() , binary_op->rvalue_in() );

        _state.commit();
        LOG_VERBOSE("Parser", OK "Binary expression parsed:\n%s\n", _state.tokens().to_string().c_str());
        return binary_op->value_out();
    }

    LOG_VERBOSE("Parser", KO "Right expression is null\n");
    _state.rollback();
    return nullptr;
}

Optional<Slot*> Nodlang::parse_unary_operator_expression(u8_t _precedence)
{
    LOG_VERBOSE("Parser", "parseUnaryOperationExpression...\n");

    if (!_state.tokens().can_eat(2))
    {
        LOG_VERBOSE("Parser", KO "Not enough tokens\n");
        return nullptr;
    }

    _state.start_transaction();
    Token operator_token = _state.tokens().eat();

    // Check if we get an operator first
    if (operator_token.m_type != Token_t::operator_)
    {
        _state.rollback();
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
        _state.rollback();
        return nullptr;
    }

    // Create a function signature
    FunctionDescriptor type;
    type.init<any(any)>(operator_token.word_to_string().c_str());
    type.arg_at(0).type = out_atomic->property->get_type();

    FunctionNode* node = _state.graph()->create_operator(type);
    node->set_identifier_token( operator_token );
    node->lvalue_in()->property->token().m_type = out_atomic->property->token().m_type;

    _state.graph()->connect_or_merge(out_atomic.get(), node->lvalue_in() );

    LOG_VERBOSE("Parser", OK "Unary expression parsed:\n%s\n", _state.tokens().to_string().c_str());
    _state.commit();

    return node->value_out();
}

Optional<Slot*> Nodlang::parse_atomic_expression()
{
    LOG_VERBOSE("Parser", "Parsing atomic expression ... \n");

    if (!_state.tokens().can_eat())
    {
        LOG_VERBOSE("Parser", KO "Not enough tokens\n");
        return nullptr;
    }

    _state.start_transaction();
    Token token = _state.tokens().eat();

    if (token.m_type == Token_t::operator_)
    {
        LOG_VERBOSE("Parser", KO "Cannot start with an operator token\n");
        _state.rollback();
        return nullptr;
    }

    if ( Optional<Slot*> result = token_to_slot(token) )
    {
        _state.commit();
        LOG_VERBOSE("Parser", OK "Atomic expression parsed:\n%s\n", _state.tokens().to_string().c_str());
        return result;
    }

    _state.rollback();
    LOG_VERBOSE( "Parser", KO "Unable to parse token (%llu)\n", token.m_index );

    return nullptr;
}

Optional<Slot*> Nodlang::parse_parenthesis_expression()
{
    LOG_VERBOSE("Parser", "parse parenthesis expr...\n");

    if (!_state.tokens().can_eat())
    {
        LOG_VERBOSE("Parser", KO "No enough tokens.\n");
        return nullptr;
    }

    _state.start_transaction();
    Token currentToken = _state.tokens().eat();
    if (currentToken.m_type != Token_t::parenthesis_open)
    {
        LOG_VERBOSE("Parser", KO "Open bracket not found.\n");
        _state.rollback();
        return nullptr;
    }

    Optional<Slot*> result = parse_expression();
    if ( result )
    {
        Token token = _state.tokens().eat();
        if (token.m_type != Token_t::parenthesis_close)
        {
            LOG_VERBOSE("Parser", "%s \n", _state.tokens().to_string().c_str());
            LOG_VERBOSE("Parser", KO "Parenthesis close expected\n",
                        token.word_to_string().c_str());
            _state.rollback();
        }
        else
        {
            LOG_VERBOSE("Parser", OK "Parenthesis expression parsed:\n%s\n", _state.tokens().to_string().c_str());
            _state.commit();
        }
    }
    else
    {
        LOG_VERBOSE("Parser", KO "No expression after open parenthesis.\n");
        _state.rollback();
    }
    return result;
}

Nodlang::FlowPath Nodlang::parse_expression_block(const FlowPathOut& flow_out, Slot* value_in )
{
    _state.start_transaction();

    // Parse an expression
    Optional<Slot*> value_out = parse_expression();

    // When expression value_out is a variable that is already part of the code flow,
    // we must create a variable reference
    if ( value_out && value_out->node->type() == NodeType_VARIABLE )
    {
        auto variable = static_cast<VariableNode*>( value_out->node );
        if ( Utils::is_connected_to_codeflow(variable) ) // in such case, we have to reference the variable, since a given variable can't be twice (be declared twice) in the codeflow
        {
            // create a new variable reference
            VariableRefNode* ref = _state.graph()->create_variable_ref();
            ref->set_variable( variable );
            // substitute value_out by variable reference's value_out
            value_out = ref->value_out();
        }
    }

    if ( !_state.tokens().can_eat() )
    {
        // we're passing here if there is no more token, which means we reached the end of file.
        // we allow an expression to end like that.
    }
    else
    {
        // However, in case there are still unparsed tokens, we expect certain type of token, otherwise we reset the result
        switch( _state.tokens().peek().m_type )
        {
            case Token_t::end_of_instruction:
            case Token_t::parenthesis_close:
                LOG_VERBOSE("Parser", "End of instruction or parenthesis close: found in next token\n");
                break;
            default:
                LOG_VERBOSE("Parser", KO "End of instruction or parenthesis close expected.\n");
                value_out.reset();
        }
    }

    // When expression value_out is null, but an input was provided,
    // we must create an empty instruction if an end_of_instruction token is found
    if (!value_out && value_in )
    {
        if (_state.tokens().peek(Token_t::end_of_instruction))
        {
            LOG_VERBOSE("Parser", "Empty expression found\n");

            Node* empty_instr = _state.graph()->create_empty_instruction();
            value_out = empty_instr->value_out();
        }
    }

    // Ensure value_out is defined or rollback transaction
    if ( !value_out )
    {
        _state.rollback();
        LOG_VERBOSE("Parser", KO "parse instruction\n");
        return {};
    }

    // Connects value_out to the provided input
    if ( value_in )
    {
        _state.graph()->connect( value_out.data(), value_in, ConnectFlag_ALLOW_SIDE_EFFECTS);
    }

    // Add an end_of_instruction token as suffix when needed
    if (Token tok = _state.tokens().eat_if(Token_t::end_of_instruction))
    {
        value_out->node->set_suffix( tok );
    }

    // Connects expression flow_in with the provided flow_out
    if ( !flow_out.empty() )
    {
        _state.graph()->connect( flow_out, value_out->node->flow_in(), ConnectFlag_ALLOW_SIDE_EFFECTS );
    }

    // Validate transaction
    _state.commit();
    LOG_VERBOSE("Parser", OK "parse instruction:\n%s\n", _state.tokens().to_string().c_str());

    return FlowPath{ value_out->node };
}

Nodlang::FlowPath Nodlang::parse_program()
{
    VERIFY(_state.graph() != nullptr, "A Graph is expected");

    _state.start_transaction();

    // Create an entry point and push its scope
    Node* entry_point = _state.graph()->create_entry_point();

    // Parse main code block
    _state.push_scope(entry_point->internal_scope() );
    FlowPath path{ entry_point };
    FlowPath block_path = parse_code_block( path.out );
    path.out = block_path.out;
    _state.pop_scope();


    // To preserve any ignored characters stored in the global token
    // we put the prefix and suffix in resp. token_begin and end.
    Token& tok = _state.tokens().global_token();
    std::string prefix = tok.prefix_to_string();
    std::string suffix = tok.suffix_to_string();
    entry_point->internal_scope()->token_begin.prefix_push_front(prefix.c_str() );
    entry_point->internal_scope()->token_end.suffix_push_back(suffix.c_str() );

    if ( _state.tokens().can_eat( ) )
    {
        _state.rollback();
        _state.graph()->clear();
        _state.graph()->on_reset.emit();
        LOG_WARNING("Parser", "Some token remains after getting an empty code block\n");
        LOG_MESSAGE("Parser", KO "Parse program.\n");
        return {};
    }
    else if (!block_path)
    {
        LOG_WARNING("Parser", "Program main block is empty\n");
    }

    _state.commit();
    _state.graph()->on_reset.emit();

    LOG_MESSAGE("Parser", OK "Parse program.\n");

    return path;
}

Nodlang::FlowPath Nodlang::parse_scoped_block(const FlowPathOut& flow_out)
{
    LOG_VERBOSE("Parser", "Parsing scoped block ...\n");

    Scope* scope = _state.current_scope();
    ASSERT(scope);
    auto scope_begin_token = _state.tokens().eat_if(Token_t::scope_begin);
    if ( !scope_begin_token )
    {
        LOG_VERBOSE("Parser", KO "Expecting main_scope begin token\n");
        return {};
    }

    _state.start_transaction();

    // Handle nested scopes
    FlowPath path = parse_code_block( flow_out ); // no return check, allows empty scope

    if ( Token scope_end_token = _state.tokens().eat_if(Token_t::scope_end) )
    {
        // Update scope's begin/end tokens
        scope->token_begin = scope_begin_token;
        scope->token_end   = scope_end_token;

        if ( !path )
        {
            Node* empty_instr = _state.graph()->create_empty_instruction();
            scope->push_back(empty_instr);
            path = empty_instr;
        }

        _state.commit();
        LOG_VERBOSE("Parser", OK "Scoped block parsed:\n%s\n", _state.tokens().to_string().c_str());
        return path;
    }
    else
    {
        LOG_VERBOSE("Parser", KO "Expecting close main_scope token\n");
    }

    scope->clear();
    _state.rollback();
    LOG_VERBOSE("Parser", KO "Scoped block parsed\n");
    return {};
}

Nodlang::FlowPath Nodlang::parse_code_block(const FlowPathOut& flow_out)
{
    LOG_VERBOSE("Parser", "Parsing code block...\n" );

    //
    // Parse n atomic code blocks
    //
    _state.start_transaction();

    FlowPath first_path;
    FlowPathOut  last_flow_out     = flow_out;
    bool     block_end_reached = false;
    size_t   block_size        = 0;

    while (_state.tokens().can_eat() && !block_end_reached )
    {
        if ( FlowPath current_path = parse_atomic_code_block(last_flow_out ) )
        {
            if ( !first_path )
                first_path = current_path;
            last_flow_out = current_path.out;
            ++block_size;
        }
        else
        {
            block_end_reached = true;
        }
    }

    FlowPath path;
    path.in  = first_path.in;
    path.out = last_flow_out;

    if ( path )
    {
        _state.commit();
        LOG_VERBOSE("Parser", OK "parse code block:\n%s\n", _state.tokens().to_string().c_str());
        return path;
    }

    _state.rollback();
    LOG_VERBOSE("Parser", KO "parse code block. Block size is %llu\n", block_size );
    return {};
}

Optional<Slot*> Nodlang::parse_expression(u8_t _precedence, Optional<Slot*> _left_override)
{
    LOG_VERBOSE("Parser", "Parsing expression ...\n");

    /*
		Get the left-handed operand
	*/
    Optional<Slot*> left = _left_override;

    if (!_state.tokens().can_eat())
    {
        LOG_VERBOSE("Parser", OK "Last token reached\n");
        return left;
    }

    if ( !left ) left = parse_parenthesis_expression();
    if ( !left ) left = parse_unary_operator_expression(_precedence);
    if ( !left ) left = parse_function_call();
    if ( !left ) left = parse_variable_declaration(); // nullptr => variable won't be attached on the codeflow, it's a part of an expression..
    if ( !left ) left = parse_atomic_expression();

    if (!_state.tokens().can_eat())
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
        if (!_state.tokens().can_eat())
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
    auto token = _state.tokens().begin();
    short int opened = 0;

    while (token != _state.tokens().end() && success)
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
                              _state.tokens().range_to_string(token->m_index, -10).c_str(),
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
    _state.reset_ribbon(const_cast<char *>(_string.data()), _string.length());
    return tokenize();
}

bool Nodlang::tokenize()
{
    LOG_MESSAGE("Parser", "Tokenization ...\n");

    size_t global_cursor       = 0;
    size_t ignored_chars_count = 0;

    while (global_cursor != _state.buffer_size() )
    {
        size_t current_cursor = global_cursor;
        Token  new_token = parse_token(_state.buffer(), _state.buffer_size(), global_cursor );

        if ( !new_token )
        {
            LOG_WARNING("Parser", KO "Unable to tokenize from \"%20s...\" (at index %llu)\n", _state.buffer_at(current_cursor), global_cursor);
            return false;
        }

        // accumulate ignored chars (see else case to know why)
        if( new_token.m_type == Token_t::ignore)
        {
            if (  _state.tokens().empty() )
            {
                _state.tokens().global_token().prefix_end_grow(new_token.length() );
                continue;
            }

            ignored_chars_count += new_token.length();
            continue;
        }

        if ( ignored_chars_count )
        {
            // case 1: if token type allows it => increase last token's prefix to wrap the ignored chars
            Token& back = _state.tokens().back();
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

        _state.tokens().push(new_token);
        LOG_VERBOSE("Parser", "%4llu) \"%s\" \n", new_token.m_index, new_token.string().c_str() );
    }

    // Append remaining ignored chars to the ribbon's suffix
    if ( ignored_chars_count )
    {
        LOG_VERBOSE("Parser", "Found ignored chars after tokenize, adding to the tokens suffix...\n");
        Token& tok = _state.tokens().global_token();
        tok.suffix_begin_grow( ignored_chars_count );
    }

    LOG_MESSAGE("Parser", OK "Tokenization.\n%s\n", _state.tokens().to_string().c_str() );

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
    if (!_state.tokens().can_eat(3))
    {
        LOG_VERBOSE("Parser", KO "3 tokens min. are required\n");
        return nullptr;
    }

    _state.start_transaction();

    // Try to parse regular function: function(...)
    std::string fct_id;
    Token token_0 = _state.tokens().eat();
    Token token_1 = _state.tokens().eat();
    if (token_0.m_type == Token_t::identifier &&
        token_1.m_type == Token_t::parenthesis_open)
    {
        fct_id = token_0.word_to_string();
        LOG_VERBOSE("Parser", OK "Regular function pattern detected.\n");
    }
    else// Try to parse operator like (ex: operator==(..,..))
    {
        Token token_2 = _state.tokens().eat();// eat a "supposed open bracket>

        if (token_0.m_type == Token_t::keyword_operator && token_1.m_type == Token_t::operator_ && token_2.m_type == Token_t::parenthesis_open)
        {
            fct_id = token_1.word_to_string();// operator
            LOG_VERBOSE("Parser", OK "Operator function-like pattern detected.\n");
        }
        else
        {
            LOG_VERBOSE("Parser", KO "Not a function.\n");
            _state.rollback();
            return nullptr;
        }
    }
    std::vector<Slot*> result_slots;

    // Declare a new function prototype
    FunctionDescriptor signature;
    signature.init<any()>(fct_id.c_str());

    bool parsingError = false;
    while (!parsingError && _state.tokens().can_eat() &&
           _state.tokens().peek().m_type != Token_t::parenthesis_close)
    {
        Optional<Slot*> expression_out = parse_expression();
        if ( expression_out )
        {
            result_slots.push_back( expression_out.get() );
            signature.push_arg( expression_out->property->get_type() );
            _state.tokens().eat_if(Token_t::list_separator);
        }
        else
        {
            parsingError = true;
        }
    }

    // eat "close bracket supposed" token
    if ( !_state.tokens().eat_if(Token_t::parenthesis_close) )
    {
        LOG_WARNING("Parser", KO "Expecting parenthesis close\n");
        _state.rollback();
        return nullptr;
    }


    // Find the prototype in the language library
    FunctionNode* fct_node = _state.graph()->create_function( signature );

    for ( int i = 0; i < fct_node->get_arg_slots().size(); i++ )
    {
        // Connects each results to the corresponding input
        _state.graph()->connect_or_merge(result_slots.at(i), fct_node->get_arg_slot(i) );
    }

    _state.commit();
    LOG_VERBOSE("Parser", KO "Function call parsed:\n%s\n", _state.tokens().to_string().c_str() );

    return fct_node->value_out();
}

Nodlang::FlowPath Nodlang::parse_if_block(const FlowPathOut& flow_out)
{
    _state.start_transaction();

    Token if_token = _state.tokens().eat_if(Token_t::keyword_if);
    if ( !if_token )
    {
        return {};
    }

    LOG_VERBOSE("Parser", "Parsing conditional structure...\n");

    bool    result = false;
    IfNode* if_node;

    Nodlang::FlowPath path;

    if_node = _state.graph()->create_cond_struct();
    _state.graph()->connect( flow_out, if_node->flow_in(), ConnectFlag_ALLOW_SIDE_EFFECTS );

    Scope* if_scope = if_node->internal_scope();
    _state.push_scope(if_scope);

    if_node->token_if  = _state.tokens().get_eaten();

    if (_state.tokens().eat_if(Token_t::parenthesis_open) )
    {
        LOG_VERBOSE("Parser", "Parsing conditional structure's condition...\n");

        // condition
        parse_expression_block(FlowPathOut{}, if_node->condition_in());

        if (_state.tokens().eat_if(Token_t::parenthesis_close) )
        {
            path.in = if_node->flow_in();

            // scope
            _state.push_scope(if_scope->partition_at(Branch_TRUE) );
            FlowPathOut branch_flow_out{if_node->branch_out(Branch_TRUE) };
            Nodlang::FlowPath block = parse_atomic_code_block( branch_flow_out );
            _state.pop_scope();

            if ( block )
            {
                for (auto _flow_out : block.out )
                    path.out.insert( _flow_out );

                // else
                Scope* false_scope = if_scope->partition_at(Branch_FALSE);
                if ( _state.tokens().eat_if(Token_t::keyword_else) )
                {
                    if_node->token_else = _state.tokens().get_eaten();

                    _state.push_scope(false_scope );
                    branch_flow_out = { if_node->branch_out(Branch_FALSE) };
                    Nodlang::FlowPath else_block;

                    if ( else_block = parse_atomic_code_block( branch_flow_out) )
                    {
                        for (auto _flow_out : else_block.out )
                            path.out.insert( _flow_out );
                        result = true;
                        LOG_VERBOSE("Parser", OK "else block parsed.\n");
                    }
                    else
                    {
                        LOG_VERBOSE("Parser", KO "Single instruction or main_scope expected\n");
                    }

                    _state.pop_scope();
                }
                else
                {
                    false_scope->token_begin = {Token_t::ignore};
                    false_scope->token_end   = {Token_t::ignore};
                    path.out.insert(if_node->branch_out(Branch_FALSE) );
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
    _state.pop_scope();

    if ( result )
    {
        _state.commit();
        LOG_VERBOSE("Parser", OK "Parse conditional structure:\n%s\n", _state.tokens().to_string().c_str() );
        return path;
    }

    _state.graph()->destroy( if_node );
    _state.rollback();
    LOG_VERBOSE("Parser", KO "Parse conditional structure \n");

    return {};
}

Nodlang::FlowPath Nodlang::parse_for_block(const FlowPathOut& flow_out)
{
    bool         success  = false;
    ForLoopNode* for_node = nullptr;
    FlowPath     path;

    _state.start_transaction();

    if ( Token token_for = _state.tokens().eat_if(Token_t::keyword_for) )
    {

        LOG_VERBOSE("Parser", "Parsing for loop ...\n");

        for_node = _state.graph()->create_for_loop();
        _state.graph()->connect( flow_out, for_node->flow_in(), ConnectFlag_ALLOW_SIDE_EFFECTS );

        for_node->token_for = token_for;

        path.in  = for_node->flow_in();
        path.out = {for_node->branch_out(Branch_FALSE)};

        Token open_bracket = _state.tokens().eat_if(Token_t::parenthesis_open);
        if ( open_bracket)
        {
            LOG_VERBOSE("Parser", "Parsing for reset_name/condition/iter instructions ...\n");

            _state.push_scope(for_node->internal_scope() );

            // first we parse three instructions, no matter if we find them, we'll continue (we are parsing something abstract)

            // parse init; condition; iteration or nothing
            const FlowPathOut none{};
            parse_expression_block(none, for_node->initialization_slot())
            && parse_expression_block(none,for_node->condition_in())
            && parse_expression_block(none, for_node->iteration_slot());

            // parse parenthesis close
            if ( Token parenthesis_close = _state.tokens().eat_if(Token_t::parenthesis_close) )
            {
                _state.push_scope(for_node->internal_scope()->partition_at(Branch_TRUE) );
                FlowPathOut branch_flow_out = {for_node->branch_out(Branch_TRUE) };
                FlowPath block = parse_atomic_code_block(branch_flow_out) ;
                _state.pop_scope();

                if ( block )
                {
                    for(auto each : block.out)
                        path.out.insert( each );

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
            _state.pop_scope();
        }
        else
        {
            LOG_VERBOSE("Parser", KO "Open parenthesis was expected.\n");
        }
    }

    if ( success )
    {
        LOG_VERBOSE("Parser", KO "For block parsed\n");
        _state.commit();
        return path;
    }

    _state.rollback();
    _state.graph()->destroy( for_node );
    LOG_VERBOSE("Parser", KO "Could not parse for block\n");
    return {};
}

Nodlang::FlowPath Nodlang::parse_while_block( const FlowPathOut& flow_out )
{
    bool           success    = false;
    WhileLoopNode* while_node = nullptr;
    FlowPath       path;

    _state.start_transaction();

    if ( Token token_while = _state.tokens().eat_if(Token_t::keyword_while) )
    {
        LOG_VERBOSE("Parser", "Parsing while ...\n");

        while_node = _state.graph()->create_while_loop();
        _state.graph()->connect( flow_out, while_node->flow_in(), ConnectFlag_ALLOW_SIDE_EFFECTS );

        while_node->token_while = token_while;
        path.in = while_node->flow_in();
        path.out = {while_node->branch_out(Branch_FALSE)};
        _state.push_scope(while_node->internal_scope() );

        if ( Token open_bracket = _state.tokens().eat_if(Token_t::parenthesis_open) )
        {
            LOG_VERBOSE("Parser", "Parsing while condition ... \n");

            // Parse an optional condition
            parse_expression_block({}, while_node->condition_in());

            if (_state.tokens().eat_if(Token_t::parenthesis_close) )
            {
                _state.push_scope(while_node->internal_scope()->partition_at(Branch_TRUE) );
                const FlowPathOut branch_flow_out = {while_node->branch_out(Branch_TRUE) };
                FlowPath block = parse_atomic_code_block( branch_flow_out );
                _state.pop_scope();

                if ( block)
                {
                    for(auto each : block.out)
                        path.out.insert( each );
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
        _state.pop_scope();
    }

    if ( success )
    {
        LOG_VERBOSE("Parser", "Parsing while:\n%s\n", _state.tokens().to_string().c_str() );
        _state.commit();
        return path;
    }

    _state.rollback();
    _state.graph()->destroy( while_node );

    return {};
}

Optional<Slot*> Nodlang::parse_variable_declaration()
{
    if (!_state.tokens().can_eat(2))
    {
        return nullptr;
    }

    _state.start_transaction();

    bool  success          = false;
    Token type_token       = _state.tokens().eat();
    Token identifier_token = _state.tokens().eat();

    if (type_token.is_keyword_type() && identifier_token.m_type == Token_t::identifier)
    {
        const TypeDescriptor* type = get_type(type_token.m_type);
        VariableNode* variable_node = _state.graph()->create_variable(type, identifier_token.word_to_string() );
        variable_node->set_flags(VariableFlag_DECLARED);
        variable_node->set_type_token( type_token );
        variable_node->set_identifier_token( identifier_token );

        // declaration with assignment ?
        Token operator_token = _state.tokens().eat_if(Token_t::operator_);
        if (operator_token && operator_token.word_len() == 1 && *operator_token.word() == '=')
        {
            // an expression is expected
            if ( Optional<Slot*> expression_out = parse_expression() )
            {
                // expression's out ----> variable's in
                _state.graph()->connect_to_variable(expression_out.get(), variable_node );

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
                        variable_node->value()->get_type()->name(),
                        identifier_token.word_to_string().c_str());
            _state.commit();
            return variable_node->value_out();
        }

        LOG_VERBOSE("Parser", KO "Initialization expression expected for %s\n", identifier_token.word_to_string().c_str());
        _state.graph()->destroy(variable_node );
    }

    _state.rollback();
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
        int precedence = get_precedence(&_node->get_func_type());

        switch ( _node->get_func_type().arg_count() )
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
        serialize_func_call(_out, &_node->get_func_type(), _node->get_arg_slots());
    }

    return _node->value_out();
}

std::string &Nodlang::serialize_func_call(std::string &_out, const FunctionDescriptor *_signature, const std::vector<Slot*> &inputs) const
{
    _out.append( _signature->get_identifier() );
    serialize_default_buffer(_out, Token_t::parenthesis_open);

    for (const Slot* input_slot : inputs)
    {
        ASSERT( input_slot->has_flags(SlotFlag_INPUT) );
        if ( input_slot != inputs.front())
        {
            serialize_default_buffer(_out, Token_t::list_separator);
        }
        serialize_input( _out, input_slot, SerializeFlag_RECURSE );
    }

    serialize_default_buffer(_out, Token_t::parenthesis_close);
    return _out;
}

std::string &Nodlang::serialize_invokable_sig(std::string &_out, const IInvokable* _invokable) const
{
    return serialize_func_sig(_out, _invokable->get_sig());
}

std::string &Nodlang::serialize_func_sig(std::string &_out, const FunctionDescriptor *_signature) const
{
    serialize_type(_out, _signature->return_type());
    _out.append(" ");
    _out.append(_signature->get_identifier());
    serialize_default_buffer(_out, Token_t::parenthesis_open);

    auto args = _signature->arg();
    for (auto it = args.begin(); it != args.end(); it++)
    {
        if (it != args.begin())
        {
            serialize_default_buffer(_out, Token_t::list_separator);
            _out.append(" ");
        }
        serialize_type(_out, it->type);
    }

    serialize_default_buffer(_out, Token_t::parenthesis_close);
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
        serialize_default_buffer(_out, Token_t::parenthesis_open);

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
        serialize_default_buffer(_out, Token_t::parenthesis_close);

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
            serialize_scope(_out, node->internal_scope() );
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
    for(Node* node : scope->child() )
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

    return _out.append(_token.begin(), _token.length());
}

std::string& Nodlang::serialize_graph(std::string &_out, const Graph* graph ) const
{
    if ( const Scope* scope = graph->root()->internal_scope() )
        serialize_scope(_out, scope);
    else
        LOG_ERROR("Serializer", "a root child is expected to serialize the graph\n");
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
    serialize_default_buffer(_out, Token_t::parenthesis_open);
    {
        const Slot* init_slot = _for_loop->find_slot_by_property_name( INITIALIZATION_PROPERTY, SlotFlag_INPUT );
        const Slot* cond_slot = _for_loop->find_slot_by_property_name( CONDITION_PROPERTY, SlotFlag_INPUT );
        const Slot* iter_slot = _for_loop->find_slot_by_property_name( ITERATION_PROPERTY, SlotFlag_INPUT );
        serialize_input( _out, init_slot, SerializeFlag_RECURSE );
        serialize_input( _out, cond_slot, SerializeFlag_RECURSE );
        serialize_input( _out, iter_slot, SerializeFlag_RECURSE );
    }
    serialize_default_buffer(_out, Token_t::parenthesis_close);
    serialize_scope(_out, _for_loop->internal_scope()->partition_at(Branch_TRUE) );

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

    if ( const Scope* branch_scope = _while_loop_node->internal_scope()->partition_at(Branch_TRUE) )
    {
        serialize_scope(_out, branch_scope);
    }

    return _out;
}


std::string& Nodlang::serialize_cond_struct(std::string &_out, const IfNode* if_node ) const
{
    // if
    serialize_token(_out, if_node->token_if);

    // condition
    SerializeFlags flags = SerializeFlag_RECURSE
                         | SerializeFlag_WRAP_WITH_BRACES;
    serialize_input(_out, if_node->condition_in(), flags );

    // scope when condition is true
    serialize_scope(_out, if_node->internal_scope()->partition_at(Branch_TRUE) );

    // when condition is false
    serialize_token(_out, if_node->token_else);
    serialize_scope(_out, if_node->internal_scope()->partition_at(Branch_FALSE) );

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
    if (!find_operator(_invokable->get_sig()->get_identifier(), static_cast<Operator_t>(_invokable->get_sig()->arg_count())))
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

std::string& Nodlang::serialize_default_buffer(std::string& _out, Token_t _token_t) const
{
    switch (_token_t)
    {
        case Token_t::end_of_line:     return _out.append("\n"); // TODO: handle all platforms
        case Token_t::operator_:       return _out.append("operator");
        case Token_t::identifier:      return _out.append("identifier");
        case Token_t::literal_string:  return _out.append("\"\"");
        case Token_t::literal_double:  return _out.append("0.0");
        case Token_t::literal_int:     return _out.append("0");
        case Token_t::literal_bool:    return _out.append("false");
        case Token_t::literal_any:     return _out.append("0");
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

int Nodlang::get_precedence( const tools::FunctionDescriptor* _func_type) const
{
    if (!_func_type)
        return std::numeric_limits<int>::min(); // default

    const Operator* operator_ptr = find_operator(_func_type->get_identifier(), static_cast<Operator_t>(_func_type->arg_count()));

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

Nodlang::FlowPath Nodlang::parse_atomic_code_block(const FlowPathOut& flow_out)
{
    LOG_VERBOSE("Parser", "Parsing atomic code block ..\n");
    ASSERT(!flow_out.empty());

    FlowPath path;

    // most common case
         if ( path = parse_scoped_block( flow_out ) );
    else if ( path = parse_expression_block( flow_out ) );
    else if ( path = parse_if_block( flow_out ) );
    else if ( path = parse_for_block( flow_out ) );
    else if ( path = parse_while_block( flow_out ) ) ;
    else      path = parse_empty_block( flow_out);

    if ( path )
    {
        if ( Token tok = _state.tokens().eat_if(Token_t::end_of_instruction) )
        {
            path.in->node->set_suffix(tok );
        }

        LOG_VERBOSE("Parser", OK "Block found (class %s)\n", path.in->node->get_class()->name() );
        return path;
    }

    LOG_VERBOSE("Parser", KO "No block found\n");
    return path;
}

std::string& Nodlang::serialize_literal(std::string &_out, const LiteralNode* node) const
{
    return serialize_property( _out, node->value() );
}

std::string& Nodlang::serialize_empty_instruction(std::string &_out, const Node* node) const
{
    return serialize_token(_out, node->value()->token() );
}

Nodlang::FlowPath Nodlang::parse_empty_block(const Nodlang::FlowPathOut& flow_out)
{
    if ( _state.tokens().peek(Token_t::end_of_instruction) )
    {
        Node* node = _state.graph()->create_empty_instruction();
        _state.graph()->connect( flow_out, node->flow_in(), ConnectFlag_ALLOW_SIDE_EFFECTS);
        return FlowPath{ node };
    }
    return {};
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
    VERIFY(g_language, "No language found, did you call init_language?");
    return g_language;
}

void ndbl::shutdown_language(Nodlang* _language)
{
    ASSERT(g_language == _language); // singleton for now
    ASSERT(g_language != nullptr);
    delete g_language;
    g_language = nullptr;
}

