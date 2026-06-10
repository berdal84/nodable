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
#include <cctype> // isdigit, isalpha, and isalnum.

#include "core/Constants.h"
#include "core/reflection/Invokable.h"
#include "core/reflection/Operator.h"
#include "tools/core/Format.h"
#include "tools/core/Log.h"
#include "tools/core/Hash.h"

#include "ndbl/core/Node.h"
#include "ndbl/core/Node_Property.h"
#include "ndbl/core/Graph.h"
#include "ndbl/core/Scope.h"

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
        { '(',  Token_Type::parenthesis_open},
        { ')',  Token_Type::parenthesis_close},
        { '{',  Token_Type::scope_begin},
        { '}',  Token_Type::scope_end},
        { '\n', Token_Type::ignore},
        { '\t', Token_Type::ignore},
        { ' ',  Token_Type::ignore},
        { ';',  Token_Type::end_of_instruction},
        { ',',  Token_Type::list_separator}
    };

    m_definition.keywords =
    {
         { "if",       Token_Type::keyword_if },
         { "for",      Token_Type::keyword_for },
         { "while",    Token_Type::keyword_while },
         { "else",     Token_Type::keyword_else },
         { "true",     Token_Type::literal_bool },
         { "false",    Token_Type::literal_bool },
         { "operator", Token_Type::keyword_operator },
    };

    m_definition.types =
    {
         { "bool",   Token_Type::keyword_bool,   type::get<bool>()},
         { "string", Token_Type::keyword_string, type::get<std::string>()},
         { "double", Token_Type::keyword_double, type::get<double>()},
         { "i16",    Token_Type::keyword_i16,    type::get<i16_t>()},
         { "int",    Token_Type::keyword_int,    type::get<i32_t>()},
         { "any",    Token_Type::keyword_any,    type::get<any>()},
         // we don't really want to parse/serialize that
         // { "unknown",Token_t::keyword_unknown,type::get<unknown>()},
    };

    m_definition.operators =
    {
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
        m_token_t_by_keyword.insert({Hash::hash(keyword), token_t});
        m_keyword_by_token_t.insert({token_t, keyword});
    }

    for( auto [keyword, token_t, type] : m_definition.types)
    {
        m_keyword_by_token_t.insert({token_t, keyword});
        m_keyword_by_type_id.insert({type->id(), keyword});
        m_token_t_by_keyword.insert({Hash::hash(keyword), token_t});
        m_token_t_by_type_id.insert({type->id(), token_t});
        m_type_by_token_t.insert({token_t, type});
    }

    for( auto [keyword, operator_t, precedence] : m_definition.operators)
    {
        const Operator *op = new Operator(keyword, operator_t, precedence);
        ASSERT(std::find(m_operators.begin(), m_operators.end(), op) == m_operators.end());
        m_operators.push_back(op);
    }
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
    _state.reset(graph_out);

    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "Parsing ...\n%s\n", code.c_str());

    if ( !tokenize(code) )
    {
        return false;
    }

    if (!is_syntax_valid())
    {
        return false;
    }

    Scope* scope = parse_program();

    if ( scope->empty(Scope_Flag_RECURSE_CHILD_PARTITION) )
    {
        return false;
    }

    if (_state.tokens().can_eat() )
    {
        _state.graph()->reset();
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " End of token ribbon expected\n");
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "%s", Format::title("Token_Ribbon").c_str());
        for (const Token& each_token : _state.tokens() )
        {
            TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "token idx %i: %s\n", each_token.m_index, each_token.json().c_str());
        }
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "%s", Format::title("Token_Ribbon end").c_str());
        auto curr_token = _state.tokens().peek();
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Failed to parse from token %llu/%llu and above.\n", curr_token.m_index, _state.tokens().size());
        TOOLS_LOG(tools::Verbosity_Error, "Parser", "Unable to parse all the tokens\n");
        return false;
    }
    return true;
}

bool Nodlang::parse_bool_or(const std::string &_str, bool default_value) const
{
    size_t cursor = 0;
    Token  token  = parse_token(_str.c_str(), _str.size(), cursor);
    if (token.m_type == Token_Type::literal_bool )
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
    if (token.m_type == Token_Type::literal_double )
        return std::stod(_str);
    return default_value;
}


int Nodlang::parse_int_or(const std::string &_str, int default_value) const
{
    size_t cursor = 0;
    Token  token  = parse_token(_str.c_str(), _str.size(), cursor);
    if (token.m_type == Token_Type::literal_int )
        return stoi(_str);
    return default_value;
}

Node_Slot* Nodlang::token_to_slot(Scope* parent_scope, const Token& _token)
{
    if (_token.m_type == Token_Type::identifier)
    {
        std::string identifier = _token.word_to_string();
        if( Node* existing = parent_scope->find_variable(identifier) )
        {
            return existing->variable_data().ref_out;
        }

        if ( !m_strict_mode )
        {
            // Insert a VariableNodeRef with "any" type
            TOOLS_LOG(tools::Verbosity_Warning,  "Parser", "%s is not declared (strict mode), abstract graph can be generated but compilation will fail.\n",
                         _token.word_to_string().c_str() );
            Node* ref = _state.graph()->create_variable_ref( parent_scope );
            ref->value->token = _token;
            return ref->value_out();
        }

        TOOLS_LOG(tools::Verbosity_Error,  "Parser", "%s is not declared (strict mode) \n", _token.word_to_string().c_str() );
        return nullptr;
    }

    Node* literal = nullptr;

    switch (_token.m_type)
    {
        case Token_Type::literal_bool:   literal = _state.graph()->create_literal<bool>( parent_scope );        break;
        case Token_Type::literal_int:    literal = _state.graph()->create_literal<i32_t>( parent_scope );       break;
        case Token_Type::literal_double: literal = _state.graph()->create_literal<double>( parent_scope );      break;
        case Token_Type::literal_string: literal = _state.graph()->create_literal<std::string>( parent_scope ); break;
        default:
            break; // we don't want to throw
    }

    if ( literal )
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " Token %s converted to a Literal %s\n",
                    _token.word_to_string().c_str(),
                    literal->value->type->name());
        literal->value->token = _token;
        return literal->value_out();
    }

    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Unable to run token_to_slot with token %s!\n", _token.word_to_string().c_str());
    return nullptr;
}

Node_Slot* Nodlang::parse_binary_operator_expression(Scope* parent_scope, u8_t _precedence, Node_Slot* _left)
{
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "Parsing binary expression ...\n");
    ASSERT(_left != nullptr);

    if (!_state.tokens().can_eat(2))
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Not enough tokens\n");
        return nullptr;
    }

    _state.start_transaction();
    const Token operator_token = _state.tokens().eat();
    const Token operand_token  = _state.tokens().peek();

    // Structure check
    const bool isValid = operator_token.m_type == Token_Type::operator_ &&
                         operand_token.m_type != Token_Type::operator_;

    if (!isValid)
    {
        _state.rollback();
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Unexpected tokens\n");
        return nullptr;
    }

    std::string word = operator_token.word_to_string();  // FIXME: avoid std::string copy, use hash
    const Operator *ope = find_operator(word, Operator_Type::Binary);
    if (ope == nullptr)
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Operator %s not found\n", word.c_str());
        _state.rollback();
        return nullptr;
    }

    // Precedence check
    if (ope->precedence <= _precedence && _precedence > 0)
    {// always update the first operation if they have the same precedence or less.
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Has lower precedence\n");
        _state.rollback();
        return nullptr;
    }

    // Parse right expression
    if ( Node_Slot* right = parse_expression(parent_scope, ope->precedence) )
    {
        // Create a function signature according to ltype, rtype and operator word
        Function_Descriptor type;
        type.init<any(any, any)>(ope->identifier.c_str());
        type.arg_at(0).type = _left->property->type;
        type.arg_at(1).type = right->property->type;

        Node* binary_op_node = _state.graph()->create_operator( type, _left->node->scope );

        auto& binary_op = binary_op_node->invokable_data();

        binary_op.set_identifier_token(operator_token);
        binary_op.lvalue_in()->property->token.m_type = _left->property->token.m_type;
        binary_op.rvalue_in()->property->token.m_type = right->property->token.m_type;

        _state.graph()->connect_or_merge(_left, binary_op.lvalue_in());
        _state.graph()->connect_or_merge(right, binary_op.rvalue_in() );

        _state.commit();
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " Binary expression parsed:\n%s\n", _state.tokens().to_string().c_str());
        return binary_op_node->value_out();
    }

    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Right expression is null\n");
    _state.rollback();
    return nullptr;
}

Node_Slot* Nodlang::parse_unary_operator_expression(Scope* parent_scope, u8_t _precedence)
{
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "parseUnaryOperationExpression...\n");

    if (!_state.tokens().can_eat(2))
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Not enough tokens\n");
        return nullptr;
    }

    _state.start_transaction();
    Token operator_token = _state.tokens().eat();

    // Check if we get an operator first
    if (operator_token.m_type != Token_Type::operator_)
    {
        _state.rollback();
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Expecting an operator token first\n");
        return nullptr;
    }

    // Parse expression after the operator
    Node_Slot* out_atomic = parse_atomic_expression(parent_scope);

    if ( !out_atomic )
    {
        out_atomic = parse_parenthesis_expression( parent_scope );
    }

    if ( !out_atomic )
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Right expression is null\n");
        _state.rollback();
        return nullptr;
    }

    // Create a function signature
    Function_Descriptor type;
    type.init<any(any)>(operator_token.word_to_string().c_str());
    type.arg_at(0).type = out_atomic->property->type;

    Node* node = _state.graph()->create_operator( type, parent_scope );
    node->invokable_data().set_identifier_token( operator_token );
    node->invokable_data().lvalue_in()->property->token.m_type = out_atomic->property->token.m_type;

    _state.graph()->connect_or_merge(out_atomic, node->invokable_data().lvalue_in() );

    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " Unary expression parsed:\n%s\n", _state.tokens().to_string().c_str());
    _state.commit();

    return node->value_out();
}

Node_Slot* Nodlang::parse_atomic_expression(Scope* parent_scope)
{
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "Parsing atomic expression ... \n");

    if (!_state.tokens().can_eat())
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Not enough tokens\n");
        return nullptr;
    }

    _state.start_transaction();
    Token token = _state.tokens().eat();

    if (token.m_type == Token_Type::operator_)
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Cannot start with an operator token\n");
        _state.rollback();
        return nullptr;
    }

    if ( Node_Slot* result = token_to_slot(parent_scope, token) )
    {
        _state.commit();
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " Atomic expression parsed:\n%s\n", _state.tokens().to_string().c_str());
        return result;
    }

    _state.rollback();
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic,  "Parser", TOOLS_KO " Unable to parse token (%llu)\n", token.m_index );

    return nullptr;
}

Node_Slot* Nodlang::parse_parenthesis_expression(Scope* parent_scope)
{
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "parse parenthesis expr...\n");

    if (!_state.tokens().can_eat())
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " No enough tokens.\n");
        return nullptr;
    }

    _state.start_transaction();
    Token currentToken = _state.tokens().eat();
    if (currentToken.m_type != Token_Type::parenthesis_open)
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Open bracket not found.\n");
        _state.rollback();
        return nullptr;
    }

    Node_Slot* result = parse_expression(parent_scope);
    if ( result )
    {
        Token token = _state.tokens().eat();
        if (token.m_type != Token_Type::parenthesis_close)
        {
            TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "%s \n", _state.tokens().to_string().c_str());
            TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Parenthesis close expected\n",
                        token.word_to_string().c_str());
            _state.rollback();
        }
        else
        {
            TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " Parenthesis expression parsed:\n%s\n", _state.tokens().to_string().c_str());
            _state.commit();
        }
    }
    else
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " No expression after open parenthesis.\n");
        _state.rollback();
    }
    return result;
}

Node* Nodlang::parse_expression_block(Scope* parent_scope, Node_Slot* flow_out, Node_Slot* value_in)
{
    _state.start_transaction();

    // Parse an expression
    Node_Slot* value_out = parse_expression(parent_scope);

    // When expression value_out is a variable that is already part of the code flow,
    // we must create a variable reference
    if ( value_out && value_out->node->type == Node_Type_VARIABLE )
    {
        Node* variable = value_out->node;

        if ( node_is_connected_to_codeflow(variable) ) // in such case, we have to reference the variable, since a given variable can't be twice (be declared twice) in the codeflow
        {
            // create a new variable reference
            Node* ref_node = _state.graph()->create_variable_ref( parent_scope );
            node_variable_ref_set_variable( ref_node, variable );
            // substitute value_out by variable reference's value_out
            value_out = ref_node->value_out();
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
            case Token_Type::end_of_instruction:
            case Token_Type::parenthesis_close:
                TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "End of instruction or parenthesis close: found in next token\n");
                break;
            default:
                TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " End of instruction or parenthesis close expected.\n");
                value_out = nullptr;
        }
    }

    // When expression value_out is null, but an input was provided,
    // we must create an empty instruction if an end_of_instruction token is found
    if (!value_out && value_in )
    {
        if (_state.tokens().peek(Token_Type::end_of_instruction))
        {
            TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "Empty expression found\n");

            Node* empty_instr = _state.graph()->create_empty_instruction( parent_scope );
            value_out = empty_instr->value_out();
        }
    }

    // Ensure value_out is defined or rollback transaction
    if ( !value_out )
    {
        _state.rollback();
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " parse instruction\n");
        return nullptr;
    }

    // Connects value_out to the provided input
    if ( value_in )
    {
        _state.graph()->connect( value_out, value_in, Graph_Flag_ALLOW_SIDE_EFFECTS);
    }

    // Add an end_of_instruction token as suffix when needed
    if (Token tok = _state.tokens().eat_if(Token_Type::end_of_instruction))
    {
        value_out->node->suffix = tok;
    }

    // Connects expression flow_in with the provided flow_out
    if ( flow_out != nullptr )
    {
        _state.graph()->connect( flow_out, value_out->node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );
    }

    // Validate transaction
    _state.commit();
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " parse instruction:\n%s\n", _state.tokens().to_string().c_str());

    return value_out->node;
}

Scope* Nodlang::parse_program()
{
    VERIFY(_state.graph() != nullptr, "A Graph is expected");

    _state.start_transaction();

    Scope* scope = _state.graph()->root_scope();

    // Parse main code block
    Node* block_last_node = parse_code_block( scope, scope->node()->flow_enter() );

    // To preserve any ignored characters stored in the global token
    // we put the prefix and suffix in resp. token_begin and end.
    Token& tok = _state.tokens().global_token();
    std::string prefix = tok.prefix_to_string();
    std::string suffix = tok.suffix_to_string();
    scope->token_begin.prefix_push_front(prefix.c_str() );
    scope->token_end.suffix_push_back(suffix.c_str() );

    if ( _state.tokens().can_eat( ) )
    {
        _state.rollback();
        _state.graph()->reset();
        _state.graph()->signal_is_complete.emit();
        TOOLS_LOG(tools::Verbosity_Warning, "Parser", "Some token remains after getting an empty code block\n");
        TOOLS_LOG(tools::Verbosity_Message, "Parser", "Parse program [OK]\n");
        return scope;
    }
    else if ( block_last_node == nullptr )
    {
        TOOLS_LOG(tools::Verbosity_Warning, "Parser", "Program main block is empty\n");
    }

    _state.commit();
    _state.graph()->signal_is_complete.emit();

    TOOLS_LOG(tools::Verbosity_Message, "Parser", "Parse program [OK]\n");

    return scope;
}

Node* Nodlang::parse_scoped_block(Scope* parent_scope, Node_Slot* flow_out)
{
    ASSERT(parent_scope);
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "Parsing scoped block ...\n");

    Token token_begin = _state.tokens().eat_if(Token_Type::scope_begin);
    if ( !token_begin )
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Expecting root_scope begin token\n");
        return nullptr;
    }

    _state.start_transaction();

    Node* node = _state.graph()->create_scope(parent_scope);

    if ( flow_out != nullptr )
        _state.graph()->connect( flow_out, node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );


    parse_code_block(node->internal_scope, node->flow_enter()); // no return check, allows empty scope
    Token token_end = _state.tokens().eat_if(Token_Type::scope_end);

    if ( token_end )
    {
        node->internal_scope->token_begin = token_begin;
        node->internal_scope->token_end = token_end;

        _state.commit();
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " Scoped block parsed:\n%s\n", _state.tokens().to_string().c_str());
        return node;
    }
    else
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Expecting close root_scope token\n");
    }

    _state.graph()->find_and_destroy(node);
    _state.rollback();
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Scoped block parsed\n");
    return nullptr;
}

Node* Nodlang::parse_code_block(Scope* parent_scope, Node_Slot* flow_out)
{
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "Parsing code block...\n" );

    //
    // Parse n atomic code blocks
    //
    _state.start_transaction();

    Node_Slot* last_node_flow_out  = flow_out;
    bool     block_end_reached = false;
    size_t   block_size        = 0;

    while (_state.tokens().can_eat() && !block_end_reached )
    {
        if ( Node* current_block = parse_atomic_code_block(parent_scope, last_node_flow_out) )
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
        _state.commit();
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " parse code block:\n%s\n", _state.tokens().to_string().c_str());
        return last_node_flow_out->node;
    }

    _state.rollback();
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " parse code block. Block size is %llu\n", block_size );
    return nullptr;
}

Node_Slot* Nodlang::parse_expression(Scope* parent_scope, u8_t _precedence, Node_Slot* _left_override)
{
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "Parsing expression ...\n");

    /*
		Get the left-handed operand
	*/
    Node_Slot* left = _left_override;

    if (!_state.tokens().can_eat())
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " Last token reached\n");
        return left;
    }

    if ( !left ) left = parse_parenthesis_expression(parent_scope);
    if ( !left ) left = parse_unary_operator_expression(parent_scope, _precedence);
    if ( !left ) left = parse_function_call(parent_scope);
    if ( !left ) left = parse_variable_declaration(parent_scope); // nullptr => variable won't be attached on the codeflow, it's a part of an expression..
    if ( !left ) left = parse_atomic_expression(parent_scope);

    if (!_state.tokens().can_eat())
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " Last token reached\n");
        return left;
    }

    if ( !left )
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " Left side is null, we return it\n");
        return left;
    }

    /*
		Get the right-handed operand
	*/
    Node_Slot* expression_out = parse_binary_operator_expression(parent_scope, _precedence, left );
    if ( expression_out )
    {
        if (!_state.tokens().can_eat())
        {
            TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " Right side parsed, and last token reached\n");
            return expression_out;
        }
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " Right side parsed, continue with a recursive call...\n");
        return parse_expression(parent_scope, _precedence, expression_out);
    }

    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " Returning left side only\n");

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
            case Token_Type::parenthesis_open:
            {
                opened++;
                break;
            }
            case Token_Type::parenthesis_close:
            {
                if (opened <= 0)
                {
                    const size_t token_count = 10;
                    const size_t begin       = token->m_index < token_count ? 0 : token->m_index - token_count;
                    const size_t end         = token->m_index + 1;
                    TOOLS_LOG(
                        tools::Verbosity_Error,
                        "Parser",
                        "Syntax Error: Unexpected close bracket after \"... %s\" (position %llu)\n",
                        _state.tokens().range_to_string(begin, end).c_str(),
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
        TOOLS_LOG(tools::Verbosity_Error, "Parser", "Syntax Error: Bracket count mismatch, %i still opened.\n", opened);
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
    TOOLS_LOG(tools::Verbosity_Diagnostic, "Parser", "Tokenization ...\n");

    size_t global_cursor       = 0;
    size_t ignored_chars_count = 0;

    while (global_cursor != _state.buffer_size() )
    {
        size_t current_cursor = global_cursor;
        Token  new_token = parse_token(_state.buffer(), _state.buffer_size(), global_cursor );

        if ( !new_token )
        {
            TOOLS_LOG(tools::Verbosity_Warning, "Parser", TOOLS_KO " Unable to tokenize from \"%20s...\" (at index %llu)\n", _state.buffer_at(current_cursor), global_cursor);
            return false;
        }

        // accumulate ignored chars (see else case to know why)
        if(new_token.m_type == Token_Type::ignore)
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
                TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "      \"%s\" (update) \n", back.string().c_str() );
            }
            // case 2: increase prefix of the new_token up to wrap the ignored chars
            else if ( new_token )
            {
                new_token.prefix_begin_grow(ignored_chars_count);
            }
            ignored_chars_count = 0;
        }

        _state.tokens().push(new_token);
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "%4llu) \"%s\" \n", new_token.m_index, new_token.string().c_str() );
    }

    // Append remaining ignored chars to the ribbon's suffix
    if ( ignored_chars_count )
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "Found ignored chars after tokenize, adding to the tokens suffix...\n");
        Token& tok = _state.tokens().global_token();
        tok.suffix_begin_grow( ignored_chars_count );
    }

    TOOLS_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " Tokenization.\n%s\n", _state.tokens().to_string().c_str() );

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
            return Token{Token_Type::ignore, const_cast<char*>(buffer), start_pos, cursor - start_pos};
        }
    }

    // single-char
    auto single_char_found = m_token_t_by_single_char.find(first_char);
    if( single_char_found != m_token_t_by_single_char.end() )
    {
        ++global_cursor;
        const Token_Type type = single_char_found->second;
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
                return Token{Token_Type::operator_, const_cast<char*>(buffer), start_pos, cursor - start_pos};
            }
            // "="
            global_cursor++;
            return Token{Token_Type::operator_, const_cast<char*>(buffer), start_pos, 1};
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
            return Token{Token_Type::operator_, const_cast<char*>(buffer), start_pos, cursor - start_pos};
        }
    }

    // number (double)
    //     note: we accept zeros as prefix (ex: "0002.15454", or "01012")
    if ( std::isdigit(first_char) )
    {
        auto cursor = start_pos + 1;
        Token_Type type = Token_Type::literal_int;

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
            type = Token_Type::literal_double;
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
        return Token{Token_Type::literal_string, const_cast<char*>(buffer), start_pos, cursor - start_pos};
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

        Token_Type type = Token_Type::identifier;

        const auto key = Hash::hash( buffer + start_pos, cursor - start_pos );
        auto keyword_found = m_token_t_by_keyword.find( key );
        if (keyword_found != m_token_t_by_keyword.end())
        {
            // a keyword has priority over identifier
            type = keyword_found->second;
        }
        return Token{type, const_cast<char*>(buffer), start_pos, cursor - start_pos};
    }
    return Token_Type::none;
}

Node_Slot* Nodlang::parse_function_call(Scope* parent_scope)
{
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "parse function call...\n");

    // Check if the minimum token count required is available ( 0: identifier, 1: open parenthesis, 2: close parenthesis)
    if (!_state.tokens().can_eat(3))
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " 3 tokens min. are required\n");
        return nullptr;
    }

    _state.start_transaction();

    // Try to parse regular function: function(...)
    std::string fct_id;
    Token token_0 = _state.tokens().eat();
    Token token_1 = _state.tokens().eat();
    if (token_0.m_type == Token_Type::identifier &&
        token_1.m_type == Token_Type::parenthesis_open)
    {
        fct_id = token_0.word_to_string();
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " Regular function pattern detected.\n");
    }
    else// Try to parse operator like (ex: operator==(..,..))
    {
        Token token_2 = _state.tokens().eat();// eat a "supposed open bracket>

        if (token_0.m_type == Token_Type::keyword_operator && token_1.m_type == Token_Type::operator_ && token_2.m_type == Token_Type::parenthesis_open)
        {
            fct_id = token_1.word_to_string();// operator
            TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " Operator function-like pattern detected.\n");
        }
        else
        {
            TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Not a function.\n");
            _state.rollback();
            return nullptr;
        }
    }
    std::vector<Node_Slot*> result_slots;

    // Declare a new function prototype
    Function_Descriptor signature;
    signature.init<any()>(fct_id.c_str());

    bool parsingError = false;
    while (!parsingError && _state.tokens().can_eat() &&
           _state.tokens().peek().m_type != Token_Type::parenthesis_close)
    {
        Node_Slot* expression_out = parse_expression(parent_scope);
        if ( expression_out )
        {
            result_slots.push_back( expression_out );
            signature.push_arg( expression_out->property->type );
            _state.tokens().eat_if(Token_Type::list_separator);
        }
        else
        {
            parsingError = true;
        }
    }

    // eat "close bracket supposed" token
    if ( !_state.tokens().eat_if(Token_Type::parenthesis_close) )
    {
        TOOLS_LOG(tools::Verbosity_Warning, "Parser", TOOLS_KO " Expecting parenthesis close\n");
        _state.rollback();
        return nullptr;
    }


    // Find the prototype in the language library
    Node* fct_node = _state.graph()->create_function( signature, parent_scope );

    for ( int i = 0; i < fct_node->invokable_data().get_arg_slots().size; i++ )
    {
        // Connects each results to the corresponding input
        _state.graph()->connect_or_merge(result_slots.at(i), fct_node->invokable_data().get_arg_slot(i) );
    }

    _state.commit();
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Function call parsed:\n%s\n", _state.tokens().to_string().c_str() );

    return fct_node->value_out();
}

Node* Nodlang::parse_if_block(Scope* parent_scope, Node_Slot* flow_out)
{
    _state.start_transaction();

    Token if_token = _state.tokens().eat_if(Token_Type::keyword_if);
    if ( !if_token )
    {
        return nullptr;
    }

    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "Parsing conditional structure...\n");

    bool     success  = false;
    Node* if_node  = _state.graph()->create_cond_struct( parent_scope );
    if_node->switch_behavior_data().m_branch_prefix = _state.tokens().get_eaten();

    _state.graph()->connect(flow_out, if_node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );

    if (_state.tokens().eat_if(Token_Type::parenthesis_open) )
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "Parsing conditional structure's condition...\n");

        // condition
        parse_expression_block(if_node->internal_scope, nullptr, if_node->switch_behavior_data().condition_in());

        if (_state.tokens().eat_if(Token_Type::parenthesis_close) )
        {
            // scope
            Node* block = parse_atomic_code_block( if_node->internal_scope, if_node->switch_behavior_data().branch_out(Branch_TRUE) );

            if ( block )
            {
                // else
                if ( _state.tokens().eat_if(Token_Type::keyword_else) )
                {
                    if_node->switch_behavior_data().m_branch_suffix = _state.tokens().get_eaten();

                    if ( Node* else_block = parse_atomic_code_block( if_node->internal_scope, if_node->switch_behavior_data().branch_out(Branch_FALSE) ) )
                    {
                        success = true;
                        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " else block parsed.\n");
                    }
                    else
                    {
                        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Single instruction or root_scope expected\n");
                    }
                }
                else
                {
                    success = true;
                }
            }
            else
            {
                TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Single instruction or root_scope expected\n");
            }
        }
        else
        {
            TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Close bracket expected\n");
        }
    }

    if ( success )
    {
        _state.commit();
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " Parse conditional structure:\n%s\n", _state.tokens().to_string().c_str() );
        // TODO: connect true/false branches flow_out to scope flow_leave?"
        return if_node;
    }

    _state.graph()->find_and_destroy(if_node);
    _state.rollback();
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Parse conditional structure \n");

    return {};
}

Node* Nodlang::parse_for_block(Scope* parent_scope, Node_Slot* flow_out)
{
    bool        success     = false;
    Node*    for_node    = nullptr;

    _state.start_transaction();

    if ( Token token_for = _state.tokens().eat_if(Token_Type::keyword_for) )
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "Parsing for loop ...\n");

        for_node = _state.graph()->create_for_loop( parent_scope );
        for_node->switch_behavior_data().m_branch_prefix = token_for;

        _state.graph()->connect( flow_out, for_node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );

        Token open_bracket = _state.tokens().eat_if(Token_Type::parenthesis_open);
        if ( open_bracket)
        {
            TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "Parsing for set_name/condition/iter instructions ...\n");

            // first we parse three instructions, no matter if we find them, we'll continue (we are parsing something abstract)

            // parse init; condition; iteration or nothing
            parse_expression_block(for_node->internal_scope, nullptr, for_node->switch_behavior_data().initialization_slot())
            && parse_expression_block(for_node->internal_scope, nullptr, for_node->switch_behavior_data().condition_in())
            && parse_expression_block(for_node->internal_scope, nullptr, for_node->switch_behavior_data().iteration_slot());

            // parse parenthesis close
            if ( Token parenthesis_close = _state.tokens().eat_if(Token_Type::parenthesis_close) )
            {
                Node* block = parse_atomic_code_block( for_node->internal_scope, for_node->switch_behavior_data().branch_out(Branch_TRUE) ) ;

                if ( block )
                {
                    success = true;
                    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "Scope or single instruction found\n");
                }
                else
                {
                    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Scope or single instruction expected\n");
                }
            }
            else
            {
                TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Close parenthesis was expected.\n");
            }
        }
        else
        {
            TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Open parenthesis was expected.\n");
        }
    }

    if ( success )
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " For block parsed\n");
        _state.commit();
        // TODO: Should we connect true/false branches to scope's flow_leave Node_Slot?
        return for_node;
    }

    if ( for_node )
    {
        _state.graph()->find_and_destroy(for_node);
    }
    _state.rollback();
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " Could not parse for block\n");
    return {};
}

Node* Nodlang::parse_while_block(Scope* parent_scope,  Node_Slot* flow_out)
{
    bool        success     = false;
    Node*    while_node  = nullptr;
    Node*    block       = nullptr;

    _state.start_transaction();

    if ( Token token_while = _state.tokens().eat_if(Token_Type::keyword_while) )
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "Parsing while ...\n");

        while_node = _state.graph()->create_while_loop( parent_scope );
        while_node->switch_behavior_data().m_branch_prefix = token_while;

        _state.graph()->connect( flow_out, while_node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );

        if ( Token open_bracket = _state.tokens().eat_if(Token_Type::parenthesis_open) )
        {
            TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "Parsing while condition ... \n");

            // Parse an optional condition
            parse_expression_block(while_node->internal_scope, nullptr, while_node->switch_behavior_data().condition_in());

            if (_state.tokens().eat_if(Token_Type::parenthesis_close) )
            {
                block = parse_atomic_code_block( while_node->internal_scope, while_node->switch_behavior_data().branch_out(Branch_TRUE) );
                if ( block )
                {
                    success = true;
                }
                else
                {
                    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO "  Scope or single instruction expected\n");
                }
            }
            else
            {
                TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO "  Parenthesis close expected\n");
            }
        }
        else
        {
            TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO "  Parenthesis close expected\n");
        }
    }

    if ( success )
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "Parsing while:\n%s\n", _state.tokens().to_string().c_str() );
        _state.commit();
        // TODO: Should we connect true/false branches to scope's flow_leave SLot?
        return while_node;
    }

    _state.rollback();
    _state.graph()->find_and_destroy(while_node);
    _state.graph()->find_and_destroy(block);

    return {};
}

Node_Slot* Nodlang::parse_variable_declaration(Scope* parent_scope)
{
    if (!_state.tokens().can_eat(2))
    {
        return nullptr;
    }

    _state.start_transaction();

    bool  success          = false;
    Token type_token       = _state.tokens().eat();
    Token identifier_token = _state.tokens().eat();

    if (type_token.is_keyword_type() && identifier_token.m_type == Token_Type::identifier)
    {
        const Type_Descriptor* type = get_type(type_token.m_type);
        Node* variable_node = _state.graph()->create_variable( type, identifier_token.word_to_string(), parent_scope );

        auto& variable_data = variable_node->variable_data();

        variable_data.set_flags(VariableFlag_DECLARED);
        variable_data.type_token = type_token;
        node_set_identifier_token(variable_node, identifier_token );

        // declaration with assignment ?
        Token operator_token = _state.tokens().eat_if(Token_Type::operator_);
        if (operator_token && operator_token.word_len() == 1 && *operator_token.word() == '=')
        {
            // an expression is expected
            if ( Node_Slot* expression_out = parse_expression(parent_scope) )
            {
                // expression's out ----> variable's in
                _state.graph()->connect_to_variable(expression_out, variable_node );

                variable_data.operator_token = operator_token;
                success = true;
            }
            else
            {
                TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO "  Initialization expression expected for %s\n", identifier_token.word_to_string().c_str());
            }
        }
            // Declaration without assignment
        else
        {
            success = true;
        }

        if ( success )
        {
            TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " Variable declaration: %s %s\n",
                        variable_node->value->type->name(),
                        identifier_token.word_to_string().c_str());
            _state.commit();
            return variable_node->value_out();
        }

        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO "  Initialization expression expected for %s\n", identifier_token.word_to_string().c_str());
        _state.graph()->find_and_destroy(variable_node);
    }

    _state.rollback();
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------------
// [SECTION] C. Serializer --------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------

const Node_Slot* Nodlang::serialize_invokable(std::string &_out, const Node* _node) const
{
    if (_node->type == Node_Type_OPERATOR )
    {
        tools::Array_View<const Node_Slot*> args = _node->invokable_data().get_arg_slots();
        int precedence = get_precedence(_node->invokable_data().get_func_type());

        switch ( _node->invokable_data().get_func_type()->arg_count() )
        {
            case 2:
            {
                // Left part of the expression
                {
                    const Function_Descriptor* l_func_type = node_get_connected_function_type(_node, LEFT_VALUE_PROPERTY);
                    bool needs_braces = l_func_type && get_precedence(l_func_type) < precedence;
                    Serialization_Flags flags = Serialization_Flag_RECURSE
                                         | needs_braces * Serialization_Flag_WRAP_WITH_BRACES ;
                    serialize_input( _out, args[0], flags );
                }

                // Operator
                VERIFY( _node->invokable_data().get_identifier_token(), "identifier token should have been assigned in parse_function_call");
                serialize_token( _out, _node->invokable_data().get_identifier_token() );

                // Right part of the expression
                {
                    const Function_Descriptor* r_func_type = node_get_connected_function_type(_node, RIGHT_VALUE_PROPERTY);
                    bool needs_braces = r_func_type && get_precedence(r_func_type) < precedence;
                    Serialization_Flags flags = Serialization_Flag_RECURSE
                                         | needs_braces * Serialization_Flag_WRAP_WITH_BRACES ;
                    serialize_input( _out, args[1], flags );
                }
                break;
            }

            case 1:
            {
                // operator ( ... innerOperator ... )   ex:   -(a+b)

                ASSERT( _node->invokable_data().get_identifier_token() );
                serialize_token(_out, _node->invokable_data().get_identifier_token());

                bool needs_braces    = node_get_connected_function_type(_node, LEFT_VALUE_PROPERTY) != nullptr;
                Serialization_Flags flags = Serialization_Flag_RECURSE
                                     | needs_braces * Serialization_Flag_WRAP_WITH_BRACES;
                serialize_input( _out, args[0], flags );
                break;
            }
        }
    }
    else
    {
        serialize_func_call(_out, _node->invokable_data().get_func_type(), _node->invokable_data().get_arg_slots() );
    }

    return _node->value_out();
}

std::string &Nodlang::serialize_func_call(std::string &_out, const Function_Descriptor *_signature, tools::Array_View<const Node_Slot*> inputs) const
{
    _out.append( _signature->get_identifier() );
    serialize_default_buffer(_out, Token_Type::parenthesis_open);

    for (const Node_Slot* input_slot : inputs)
    {
        ASSERT( input_slot->has_flags(Node_Slot_Flag_INPUT) );
        if ( input_slot != inputs[0])
        {
            serialize_default_buffer(_out, Token_Type::list_separator);
        }
        serialize_input( _out, input_slot, Serialization_Flag_RECURSE );
    }

    serialize_default_buffer(_out, Token_Type::parenthesis_close);
    return _out;
}

std::string &Nodlang::serialize_invokable_sig(std::string &_out, const IInvokable* _invokable) const
{
    return serialize_func_sig(_out, _invokable->get_sig());
}

std::string &Nodlang::serialize_func_sig(std::string &_out, const Function_Descriptor *_signature) const
{
    serialize_type(_out, _signature->return_type());
    _out.append(" ");
    _out.append(_signature->get_identifier());
    serialize_default_buffer(_out, Token_Type::parenthesis_open);

    auto args = _signature->arg();
    for (auto it = args.begin(); it != args.end(); it++)
    {
        if (it != args.begin())
        {
            serialize_default_buffer(_out, Token_Type::list_separator);
            _out.append(" ");
        }
        serialize_type(_out, it->type);
    }

    serialize_default_buffer(_out, Token_Type::parenthesis_close);
    return _out;
}

std::string &Nodlang::serialize_type(std::string &_out, const Type_Descriptor *_type) const
{
    auto found = m_keyword_by_type_id.find(_type->id());
    if (found != m_keyword_by_type_id.cend())
    {
        return _out.append(found->second);
    }
    return _out;
}

std::string& Nodlang::serialize_variable_ref(std::string &_out, const Node* _node) const
{
    ASSERT(_node->is_variable_ref());
    return serialize_token( _out, node_get_identifier_token(_node) );
}

std::string& Nodlang::serialize_variable(std::string &_out, const Node *_node) const
{
    ASSERT(_node->is_variable());

    // 1. Serialize variable's type

    // If parsed
    if ( _node->variable_data().type_token )
    {
        serialize_token(_out, _node->variable_data().type_token);
    }
    else // If created in the graph by the user
    {
        serialize_type(_out, _node->value->type);
        _out.append(" ");
    }

    // 2. Serialize variable identifier
    serialize_token( _out, node_get_identifier_token(_node) );

    // 3. Initialisation
    //    When a VariableNode has its input connected, we serialize it as its initialisation expression

    const Node_Slot* slot = _node->value_in();
    if ( slot->adjacent_count() != 0 )
    {
        if ( _node->variable_data().operator_token )
            _out.append(_node->variable_data().operator_token.string());
        else
            _out.append(" = ");

        serialize_input( _out, slot, Serialization_Flag_RECURSE );
    }
    return _out;
}

std::string &Nodlang::serialize_input(std::string& _out, const Node_Slot* slot, Serialization_Flags _flags ) const
{
    ASSERT( slot->has_flags( Node_Slot_Flag_INPUT ) );

    const Node_Slot*     adjacent_slot     = slot->first_adjacent();
    const Node_Property* adjacent_property = adjacent_slot != nullptr ? adjacent_slot->property
                                                                        : nullptr;
    // Append open brace?
    if ( _flags & Serialization_Flag_WRAP_WITH_BRACES )
        serialize_default_buffer(_out, Token_Type::parenthesis_open);

    if ( adjacent_property == nullptr )
    {
        // Simply serialize this property
        serialize_property(_out, slot->property);
    }
    else
    {
        VERIFY( _flags & Serialization_Flag_RECURSE, "Why would you call serialize_input without RECURSE flag?");
        // Append token prefix?
        if (adjacent_property->token)
            _out.append(adjacent_property->token.prefix(), adjacent_property->token.prefix_len() );

        // Serialize adjacent slot
        serialize_value_out(_out, adjacent_slot, Serialization_Flag_RECURSE);

        // Append token suffix?
        if (adjacent_property->token )
                _out.append(adjacent_property->token.suffix(), adjacent_property->token.suffix_len() );
    }

    // Append close brace?
    if ( _flags & Serialization_Flag_WRAP_WITH_BRACES )
        serialize_default_buffer(_out, Token_Type::parenthesis_close);

    return _out;
}

std::string &Nodlang::serialize_value_out(std::string& _out, const Node_Slot* slot, Serialization_Flags _flags) const
{
    // If output is node's output value, we serialize the node
    if( slot == slot->node->value_out() )
    {
        serialize_node(_out, slot->node, _flags);
        return _out;
    }

    // Otherwise, it might be a variable reference, so we serialize the identifier only
    ASSERT(slot->node->type == Node_Type_VARIABLE ); // Can't be another type
    auto& variable = slot->node->variable_data();
    VERIFY( slot == variable.ref_out, "Cannot serialize an other slot from a VariableNode");
    return _out.append( node_get_identifier(slot->node) );
}

std::string& Nodlang::serialize_node(std::string &_out, const Node* node, Serialization_Flags _flags ) const
{
    if ( node == nullptr )
        return _out;

    ASSERT( _flags == Serialization_Flag_RECURSE ); // The only flag configuration handled for now

    switch ( node->type )
    {
        case Node_Type_IF_ELSE:           serialize_cond_struct(_out, node );             break;
        case Node_Type_FOR_LOOP:          serialize_for_loop(_out, node );                break;
        case Node_Type_WHILE_LOOP:        serialize_while_loop(_out, node );              break;
        case Node_Type_LITERAL:           serialize_literal(_out, node );                 break;
        case Node_Type_VARIABLE:          serialize_variable(_out, node );                break;
        case Node_Type_VARIABLE_REF:      serialize_variable_ref(_out, node );            break;
        case Node_Type_FUNCTION:          [[fallthrough]];        
        case Node_Type_OPERATOR:          serialize_invokable(_out, node );               break;
        case Node_Type_EMPTY_INSTRUCTION: serialize_empty_instruction(_out, node);        break;
        case Node_Type_ROOT:              [[fallthrough]];
        case Node_Type_SCOPE:             serialize_scope(_out, node->internal_scope ); break;
        default:                            VERIFY(false, "Unhandled NodeType, can't serialize");
    }
    serialize_token(_out, node->suffix );

    return _out;
}

std::string& Nodlang::serialize_scope(std::string &_out, const Scope* scope) const
{
    serialize_token(_out, scope->token_begin);
    for(Node* node : scope->backbone() )
    {
        serialize_node(_out, node, Serialization_Flag_RECURSE);
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
    if ( !graph->root_node() )
    {
        TOOLS_LOG(tools::Verbosity_Error, "Serializer", "a root primary_child is expected to serialize the graph\n");
        return _out;
    }
    return serialize_node(_out, graph->root_node(), Serialization_Flag_RECURSE);
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
    return _out.append( Format::number(d) );
}

std::string& Nodlang::serialize_for_loop(std::string &_out, const Node* _for_loop) const
{
    ASSERT( _for_loop->type == Node_Type_FOR_LOOP );

    serialize_token(_out, _for_loop->switch_behavior_data().m_branch_prefix);
    serialize_default_buffer(_out, Token_Type::parenthesis_open);
    {
        const Node_Slot* init_slot = node_find_slot_by_property_name(_for_loop, INITIALIZATION_PROPERTY, Node_Slot_Flag_INPUT );
        const Node_Slot* cond_slot = node_find_slot_by_property_name(_for_loop, CONDITION_PROPERTY, Node_Slot_Flag_INPUT );
        const Node_Slot* iter_slot = node_find_slot_by_property_name(_for_loop, ITERATION_PROPERTY, Node_Slot_Flag_INPUT );
        serialize_input( _out, init_slot, Serialization_Flag_RECURSE );
        serialize_input( _out, cond_slot, Serialization_Flag_RECURSE );
        serialize_input( _out, iter_slot, Serialization_Flag_RECURSE );
    }
    serialize_default_buffer(_out, Token_Type::parenthesis_close);
    serialize_node(_out, _for_loop->switch_behavior_data().branch_out(Branch_TRUE)->first_adjacent_node(), Serialization_Flag_RECURSE );

    return _out;
}

std::string& Nodlang::serialize_while_loop(std::string &_out, const Node* _while_loop_node) const
{
    ASSERT( _while_loop_node->type == Node_Type_WHILE_LOOP );

    // while
    serialize_token(_out, _while_loop_node->switch_behavior_data().m_branch_prefix);

    // condition
    Serialization_Flags flags = Serialization_Flag_RECURSE
                         | Serialization_Flag_WRAP_WITH_BRACES;
    serialize_input(_out, _while_loop_node->switch_behavior_data().condition_in(), flags );

    if ( const Node* _node = _while_loop_node->switch_behavior_data().branch_out(Branch_TRUE)->first_adjacent_node() )
    {
        serialize_node(_out, _node, Serialization_Flag_RECURSE);
    }

    return _out;
}


std::string& Nodlang::serialize_cond_struct(std::string &_out, const Node* if_node ) const
{
    ASSERT( if_node->type == Node_Type_IF_ELSE );

    // if
    serialize_token(_out, if_node->switch_behavior_data().m_branch_prefix );

    // condition
    Serialization_Flags flags = Serialization_Flag_RECURSE
                         | Serialization_Flag_WRAP_WITH_BRACES;
    serialize_input(_out, if_node->switch_behavior_data().condition_in(), flags );

    // when condition is true
    serialize_node(_out, if_node->switch_behavior_data().branch_out(Branch_TRUE)->first_adjacent_node(), Serialization_Flag_RECURSE );

    // when condition is false
    serialize_token(_out, if_node->switch_behavior_data().m_branch_suffix);
    serialize_node(_out, if_node->switch_behavior_data().branch_out(Branch_FALSE)->first_adjacent_node(), Serialization_Flag_RECURSE );

    return _out;
}

// Language definition ------------------------------------------------------------------------------------------------------------

std::string& Nodlang::serialize_property(std::string& _out, const Node_Property* _property) const
{
    return serialize_token(_out, _property->token);
}

const Operator *Nodlang::find_operator(const std::string &_identifier, Operator_Type operator_type) const
{
    auto is_exactly = [&](const Operator *op) {
        return op->identifier == _identifier && op->type == operator_type;
    };

    auto found = std::find_if(m_operators.cbegin(), m_operators.cend(), is_exactly);

    if (found != m_operators.end())
        return *found;

    return nullptr;
}

bool Nodlang::is_operator(const Function_Descriptor* descriptor) const
{
    switch ( descriptor->arg_count() )
    {
        case 1:
            return find_operator( descriptor->name(), tools::Operator_Type::Unary );
        case 2:
            return find_operator( descriptor->name(), tools::Operator_Type::Binary );
        default:
            return false;
    }
}

std::string& Nodlang::serialize_default_buffer(std::string& _out, Token_Type _token_t) const
{
    switch (_token_t)
    {
        case Token_Type::end_of_line:     return _out.append("\n"); // TODO: handle all platforms
        case Token_Type::operator_:       return _out.append("operator");
        case Token_Type::identifier:      return _out.append("identifier");
        case Token_Type::literal_string:  return _out.append("\"\"");
        case Token_Type::literal_double:  return _out.append("0.0");
        case Token_Type::literal_int:     return _out.append("0");
        case Token_Type::literal_bool:    return _out.append("false");
        case Token_Type::literal_any:     return _out.append("0");
        case Token_Type::ignore:          [[fallthrough]];
        case Token_Type::literal_unknown: return _out;
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

std::string Nodlang::serialize_type(const Type_Descriptor *_type) const
{
    std::string result;
    serialize_type(result, _type);
    return result;
}

int Nodlang::get_precedence( const tools::Function_Descriptor* _func_type) const
{
    if (!_func_type)
        return std::numeric_limits<int>::min(); // default

    const Operator* operator_ptr = find_operator(_func_type->get_identifier(), static_cast<Operator_Type>(_func_type->arg_count()));

    if (operator_ptr)
        return operator_ptr->precedence;
    return std::numeric_limits<int>::max();
}

const Type_Descriptor* Nodlang::get_type(Token_Type _token) const
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

bool Nodlang::accepts_suffix(Token_Type type) const
{
    return type != Token_Type::identifier          // identifiers must stay clean because they are reused
              && type != Token_Type::parenthesis_open    // ")" are lost when creating AST
              && type != Token_Type::parenthesis_close;  // "(" are lost when creating AST
}

Token_Type Nodlang::to_literal_token(const Type_Descriptor *type) const
{
    if (type == type::get<double>() )
        return Token_Type::literal_double;
    if (type == type::get<i16_t>() )
        return Token_Type::literal_int;
    if (type == type::get<int>() )
        return Token_Type::literal_int;
    if (type == type::get<bool>() )
        return Token_Type::literal_bool;
    if (type == type::get<std::string>() )
        return Token_Type::literal_string;
    if (type == type::get<any>() )
        return Token_Type::literal_any;
    return Token_Type::literal_unknown;
}

Node* Nodlang::parse_atomic_code_block(Scope* parent_scope, Node_Slot* flow_out)
{
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", "Parsing atomic code block ..\n");
    ASSERT(flow_out);

    // most common case
    Node* block = nullptr;
         if ( (block = parse_scoped_block(parent_scope, flow_out)) );
    else if ( (block = parse_expression_block(parent_scope, flow_out)) );
    else if ( (block = parse_if_block(parent_scope, flow_out)) );
    else if ( (block = parse_for_block(parent_scope, flow_out)) );
    else if ( (block = parse_while_block(parent_scope, flow_out)) ) ;
    else      (block = parse_empty_block(parent_scope, flow_out));

    if ( block )
    {
        if ( Token tok = _state.tokens().eat_if(Token_Type::end_of_instruction) )
        {
            block->suffix = tok;
        }

        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_OK " Block found (class %s)\n", block->get_class()->name() );
        return block;
    }

    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Parser", TOOLS_KO " No block found\n");
    return nullptr;
}

std::string& Nodlang::serialize_literal(std::string &_out, const Node* node) const
{
    ASSERT( node->type == Node_Type_LITERAL );
    return serialize_property( _out, node->value );
}

std::string& Nodlang::serialize_empty_instruction(std::string &_out, const Node* node) const
{
    ASSERT( node->type == Node_Type_EMPTY_INSTRUCTION );
    return serialize_token(_out, node->value->token );
}

Node* Nodlang::parse_empty_block(Scope* parent_scope, Node_Slot* flow_out)
{
    if ( _state.tokens().peek(Token_Type::end_of_instruction) )
    {
        Node* node = _state.graph()->create_empty_instruction( parent_scope );
        _state.graph()->connect( flow_out, node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS);
        return node;
    }
    return nullptr;
}

void Nodlang::ParserState::reset_graph(Graph* new_graph)
{
    new_graph->reset();
    _graph = new_graph; // memory not owned
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

bool ndbl::has_language()
{
    return g_language != nullptr;
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

