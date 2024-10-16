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
         { "if",   Token_t::keyword_if },
         { "for",  Token_t::keyword_for },
         { "while",  Token_t::keyword_while },
         { "else", Token_t::keyword_else },
         { "true", Token_t::literal_bool },
         { "false",    Token_t::literal_bool },
         { "operator", Token_t::keyword_operator },
    };

    m_definition.types =
    {
         { "bool",   Token_t::keyword_bool,   type::get<bool>()},
         { "string", Token_t::keyword_string, type::get<std::string>()},
         { "double", Token_t::keyword_double, type::get<double>()},
         { "i16",    Token_t::keyword_i16,    type::get<i16_t>()},
         { "int",    Token_t::keyword_int,    type::get<i32_t>()}
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
        ASSERT(std::find(m_operators.begin(), m_operators.end(), op) == m_operators.end())
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

void Nodlang::rollback_transaction()
{
    parser_state.ribbon.transaction_rollback();
}

void Nodlang::start_transaction()
{
    parser_state.ribbon.transaction_start();
}

void Nodlang::commit_transaction()
{
    parser_state.ribbon.transaction_commit();
}

bool Nodlang::parse(const std::string &_source_code, Graph *_graphNode)
{
    using namespace std::chrono;
    high_resolution_clock::time_point parse_begin = high_resolution_clock::now();

    parser_state.clear();
    parser_state.set_source_buffer(_source_code.c_str(), _source_code.size());
    parser_state.graph = _graphNode;

    LOG_VERBOSE("Parser", "Trying to evaluate evaluated: <expr>%s</expr>\"\n", _source_code.c_str())
    LOG_MESSAGE("Parser", "Tokenization ...\n")


    if (!tokenize(parser_state.source_buffer, parser_state.source_buffer_size))
    {
        return false;
    }

    if (!is_syntax_valid())
    {
        return false;
    }

    high_resolution_clock::time_point tokenize_end = high_resolution_clock::now();
    LOG_MESSAGE("Parser", "%16s == %.3f ms\n", "tokenize()",  duration_cast<duration<double>>( tokenize_end - parse_begin).count() * 1000.0)

    Node* program = parse_program();

    high_resolution_clock::time_point parse_program_end = high_resolution_clock::now();
    LOG_MESSAGE("Parser", "%16s == %.3f ms\n", "parse_program()", duration_cast<duration<double>>(parse_program_end - tokenize_end).count() * 1000.0)

    if ( program == nullptr )
    {
        LOG_WARNING("Parser", "Unable to generate program tree.\n")
        return false;
    }

    if (parser_state.ribbon.can_eat())
    {
        parser_state.graph->clear();
        LOG_WARNING("Parser", "Unable to generate a full program tree.\n")
        LOG_MESSAGE("Parser", "%s", format::title("TokenRibbon").c_str());
        for (const Token& each_token : parser_state.ribbon)
        {
            LOG_MESSAGE("Parser", "token idx %i: %s\n", each_token.m_index, each_token.json().c_str());
        }
        LOG_MESSAGE("Parser", "%s", format::title("TokenRibbon end").c_str());
        auto curr_token = parser_state.ribbon.peek();
        LOG_ERROR("Parser", "Couldn't parse token %llu and above: %s\n", curr_token.m_index, curr_token.json().c_str())
        return false;
    }

    // We unset dirty, since we did a lot of connections but we don't want any update now
    auto &nodes = parser_state.graph->get_node_registry();
    for (auto eachNode: nodes)
    {
        eachNode->clear_flags(NodeFlag_IS_DIRTY);
    }

    LOG_MESSAGE("Parser", "Program tree updated in %.3f ms.\n", duration_cast<duration<double>>(high_resolution_clock::now() - parse_begin).count()*1000.0 )
    LOG_VERBOSE("Parser", "Source code: <expr>%s</expr>\"\n", _source_code.c_str())

    return true;
}

bool Nodlang::to_bool(const std::string &_str)
{
    return _str == std::string("true");
}

std::string Nodlang::to_unquoted_string(const std::string &_quoted_str)
{
    ASSERT(_quoted_str.size() >= 2);
    ASSERT(_quoted_str.front() == '\"');
    ASSERT(_quoted_str.back() == '\"');
    return std::string(++_quoted_str.cbegin(), --_quoted_str.cend());
}

double Nodlang::to_double(const std::string &_str)
{
    return stod(_str);
}

int Nodlang::to_int(const std::string &_str)
{
    return stoi(_str);
}

Slot *Nodlang::parse_token(Token _token)
{
    if (_token.m_type == Token_t::identifier)
    {
        std::string identifier = _token.word_to_string();
        VariableNode* existing_variable = get_current_scope()->find_variable(identifier );

        if( existing_variable != nullptr )
            return &existing_variable->output_slot();

        if ( !m_strict_mode )
        {
            /* when strict mode is OFF, we just create a variable with Any type */
            LOG_WARNING( "Parser", "%s is not declared (strict mode), abstract graph can be generated but compilation will fail.\n",
                         _token.word_to_string().c_str() )
            existing_variable = parser_state.graph->create_variable(type::null(), _token.word_to_string(), get_current_scope() );
            existing_variable->set_identifier_token(_token );
            existing_variable->set_flags(VariableFlag_DECLARED);
            return &existing_variable->output_slot();
        }

        LOG_ERROR( "Parser", "%s is not declared (strict mode) \n", _token.word_to_string().c_str() )
        return nullptr;
    }

    LiteralNode* literal{};

    switch (_token.m_type)
    {
        case Token_t::literal_bool:   literal = parser_state.graph->create_literal<bool>();        break;
        case Token_t::literal_int:    literal = parser_state.graph->create_literal<i32_t>();       break;
        case Token_t::literal_double: literal = parser_state.graph->create_literal<double>();      break;
        case Token_t::literal_string: literal = parser_state.graph->create_literal<std::string>(); break;
        default:
            break; // we don't want to throw
    }

    if ( literal == nullptr)
    {
        LOG_VERBOSE("Parser", "Unable to perform token_to_property for token %s!\n", _token.word_to_string().c_str())
        return nullptr;
    }

    literal->value()->set_token( _token );
    return &literal->output_slot();
}

Slot *Nodlang::parse_binary_operator_expression(u8_t _precedence, Slot& _left)
{
    LOG_VERBOSE("Parser", "parse binary operation expr...\n")
    LOG_VERBOSE("Parser", "%s \n", parser_state.ribbon.to_string().c_str())

    if (!parser_state.ribbon.can_eat(2))
    {
        LOG_VERBOSE("Parser", "parse binary operation expr...... " KO " (not enought tokens)\n")
        return nullptr;
    }

    start_transaction();
    const Token operator_token = parser_state.ribbon.eat();
    const Token operand_token  = parser_state.ribbon.peek();

    // Structure check
    const bool isValid = operator_token.m_type == Token_t::operator_ &&
                         operand_token.m_type != Token_t::operator_;

    if (!isValid)
    {
        rollback_transaction();
        LOG_VERBOSE("Parser", "parse binary operation expr... " KO " (Structure)\n")
        return nullptr;
    }

    std::string word = operator_token.word_to_string();  // FIXME: avoid std::string copy, use hash
    const Operator *ope = find_operator(word, Operator_t::Binary);
    if (ope == nullptr)
    {
        LOG_VERBOSE("Parser", "parse binary operation expr... " KO " (unable to find operator %s)\n", word.c_str())
        rollback_transaction();
        return nullptr;
    }

    // Precedence check
    if (ope->precedence <= _precedence && _precedence > 0)
    {// always update the first operation if they have the same precedence or less.
        LOG_VERBOSE("Parser", "parse binary operation expr... " KO " (Precedence)\n")
        rollback_transaction();
        return nullptr;
    }


    // Parse right expression
    Slot* right = parse_expression(ope->precedence);

    if ( right == nullptr )
    {
        LOG_VERBOSE("Parser", "parseBinaryOperationExpression... " KO " (right expression is nullptr)\n")
        rollback_transaction();
        return nullptr;
    }

    // Create a function signature according to ltype, rtype and operator word
    FunctionDescriptor* type = FunctionDescriptor::create<any()>(ope->identifier.c_str());
    type->push_arg( _left.get_property()->get_type());
    type->push_arg(right->get_property()->get_type());

    FunctionNode* binary_op = parser_state.graph->create_operator(type);
    binary_op->set_identifier_token( operator_token );
    binary_op->get_lvalue()->get_property()->get_token().m_type = _left.get_property()->get_token().m_type;
    binary_op->get_rvalue()->get_property()->get_token().m_type = right->get_property()->get_token().m_type;

    parser_state.graph->connect_or_merge( _left, *binary_op->get_lvalue());
    parser_state.graph->connect_or_merge( *right, *binary_op->get_rvalue() );

    commit_transaction();
    LOG_VERBOSE("Parser", "parse binary operation expr... " OK "\n")
    return binary_op->find_slot_by_property_name( VALUE_PROPERTY, SlotFlag_OUTPUT );
}

Slot *Nodlang::parse_unary_operator_expression(u8_t _precedence)
{
    LOG_VERBOSE("Parser", "parseUnaryOperationExpression...\n")
    LOG_VERBOSE("Parser", "%s \n", parser_state.ribbon.to_string().c_str())

    if (!parser_state.ribbon.can_eat(2))
    {
        LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (not enough tokens)\n")
        return nullptr;
    }

    start_transaction();
    Token operator_token = parser_state.ribbon.eat();

    // Check if we get an operator first
    if (operator_token.m_type != Token_t::operator_)
    {
        rollback_transaction();
        LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (operator not found)\n")
        return nullptr;
    }

    // Parse expression after the operator
    Slot* out_atomic = parse_atomic_expression();

    if ( out_atomic == nullptr )
    {
        out_atomic = parse_parenthesis_expression();
    }

    if ( out_atomic == nullptr )
    {
        LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (right expression is nullptr)\n")
        rollback_transaction();
        return nullptr;
    }

    // Create a function signature
    FunctionDescriptor* type = FunctionDescriptor::create<any()>(operator_token.word_to_string().c_str());
    type->push_arg( out_atomic->get_property()->get_type());

    FunctionNode* node = parser_state.graph->create_operator(type);
    node->set_identifier_token( operator_token );
    node->get_lvalue()->get_property()->get_token().m_type = out_atomic->get_property()->get_token().m_type;

    parser_state.graph->connect_or_merge( *out_atomic, *node->find_slot_by_property_name( LEFT_VALUE_PROPERTY, SlotFlag_INPUT ) );

    LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " OK "\n")
    commit_transaction();

    return node->find_slot_by_property_name( VALUE_PROPERTY, SlotFlag_OUTPUT );
}

Slot* Nodlang::parse_atomic_expression()
{
    LOG_VERBOSE("Parser", "parse atomic expr... \n")

    if (!parser_state.ribbon.can_eat())
    {
        LOG_VERBOSE("Parser", "parse atomic expr... " KO "(not enough tokens)\n")
        return nullptr;
    }

    start_transaction();
    Token token = parser_state.ribbon.eat();

    if (token.m_type == Token_t::operator_)
    {
        LOG_VERBOSE("Parser", "parse atomic expr... " KO "(token is an operator)\n")
        rollback_transaction();
        return nullptr;
    }

    if ( Slot* result = parse_token(token) )
    {
        commit_transaction();
        LOG_VERBOSE("Parser", "parse atomic expr... " OK "\n")
        return result;
    }

    rollback_transaction();
    LOG_VERBOSE("Parser", "parse atomic expr... " KO " (result is nullptr)\n")

    return nullptr;
}

Slot *Nodlang::parse_parenthesis_expression()
{
    LOG_VERBOSE("Parser", "parse parenthesis expr...\n")
    LOG_VERBOSE("Parser", "%s \n", parser_state.ribbon.to_string().c_str())

    if (!parser_state.ribbon.can_eat())
    {
        LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " no enough tokens.\n")
        return nullptr;
    }

    start_transaction();
    Token currentToken = parser_state.ribbon.eat();
    if (currentToken.m_type != Token_t::parenthesis_open)
    {
        LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " open bracket not found.\n")
        rollback_transaction();
        return nullptr;
    }

    Slot* result = parse_expression();
    if ( result != nullptr )
    {
        Token token = parser_state.ribbon.eat();
        if (token.m_type != Token_t::parenthesis_close)
        {
            LOG_VERBOSE("Parser", "%s \n", parser_state.ribbon.to_string().c_str())
            LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " ( \")\" expected instead of %s )\n",
                        token.word_to_string().c_str())
            rollback_transaction();
        }
        else
        {
            LOG_VERBOSE("Parser", "parse parenthesis expr..." OK "\n")
            commit_transaction();
        }
    }
    else
    {
        LOG_VERBOSE("Parser", "parse parenthesis expr..." KO ", expression in parenthesis is nullptr.\n")
        rollback_transaction();
    }
    return result;
}

Node* Nodlang::parse_instr()
{
    start_transaction();

    Slot* expression_out = parse_expression();

    if ( !expression_out )
    {
        LOG_VERBOSE("Parser", "parse instruction " KO " (parsed is nullptr)\n")
        rollback_transaction();
        return {};
    }

    Node* instr_node = expression_out->get_node();

    // wraps variable as a VariableRefNode
    // This is necessary to avoid connecting multiple times the same VariableNode to a parent
    if ( instr_node->type() == NodeType_VARIABLE )
    {
        auto variable = static_cast<VariableNode*>( instr_node );
        bool is_orphan = variable->find_parent() == nullptr;
        if ( !is_orphan )
        {
            const TypeDescriptor* variable_type = variable->get_type();
            VariableRefNode* ref = parser_state.graph->create_variable_ref( variable_type );
            parser_state.graph->connect( *expression_out, *ref->get_input_slot(), ConnectFlag_ALLOW_SIDE_EFFECTS );
            instr_node = ref; // override
        }
    }

    // Handle suffix
    if (parser_state.ribbon.can_eat())
    {
        Token expected_end_of_instr_token = parser_state.ribbon.eat_if(Token_t::end_of_instruction);
        if (!expected_end_of_instr_token.is_null())
        {
            instr_node->set_suffix( expected_end_of_instr_token );
        }
        else if (parser_state.ribbon.peek().m_type != Token_t::parenthesis_close)
        {
            LOG_VERBOSE("Parser", "parse instruction " KO " (end of instruction not found)\n")
            rollback_transaction();
            return {};
        }
    }

    LOG_VERBOSE("Parser", "parse instruction " OK "\n")
    commit_transaction();

    return instr_node;
}

Node* Nodlang::parse_program()
{
    start_transaction();

    parser_state.graph->clear();
    Node* root = parser_state.graph->create_root();
    Scope* program_scope = root->get_component<Scope>();
    parser_state.scope.emplace(program_scope);

    parse_code_block();// we do not check if we parsed something empty or not, a program can be empty.

    // Add ignored chars pre/post token to the main scope begin/end token prefix/suffix.
    ASSERT(program_scope->token_begin.is_null())
    ASSERT(program_scope->token_end.is_null())
    program_scope->token_begin = parser_state.ribbon.prefix();
    program_scope->token_end = parser_state.ribbon.suffix();

    parser_state.scope.pop();
    commit_transaction();

    // Avoid an unnecessary serialization of the graph (would happen once after each parsing)
    parser_state.graph->set_dirty(false);

    return root;
}

Node* Nodlang::parse_scope( Slot& _parent_scope_slot )
{
    auto scope_begin_token = parser_state.ribbon.eat_if(Token_t::scope_begin);
    if ( !scope_begin_token )
    {
        return {};
    }

    start_transaction();

    Node*  curr_scope_node   = get_current_scope_node();
    Node*  new_scope_node    = parser_state.graph->create_scope();
    Scope* new_scope         = new_scope_node->get_component<Scope>();

    // Handle nested scopes
    parser_state.graph->connect( _parent_scope_slot, *new_scope_node->find_slot( SlotFlag_PARENT ), ConnectFlag_ALLOW_SIDE_EFFECTS );

    parser_state.scope.push( new_scope );
    parse_code_block();
    parser_state.scope.pop();

    auto scope_end_token = parser_state.ribbon.eat_if(Token_t::scope_end);
    if ( !scope_end_token )
    {
        parser_state.graph->destroy( new_scope_node );
        rollback_transaction();
        return {};
    }

    new_scope->token_begin = scope_begin_token;
    new_scope->token_end   = scope_end_token;
    commit_transaction();

    return new_scope_node;
}

void Nodlang::parse_code_block()
{
    start_transaction();

    bool block_end_reached = false;
    while ( parser_state.ribbon.can_eat() && !block_end_reached )
    {
        if ( Node* instruction = parse_instr() )
        {
            Slot* child_slot = get_current_scope_node()->find_slot( SlotFlag_CHILD | SlotFlag_NOT_FULL );
            ASSERT(child_slot)
            // Create parent/child connection
            parser_state.graph->connect(
                    *child_slot,
                    *instruction->find_slot( SlotFlag_PARENT ),
                    ConnectFlag_ALLOW_SIDE_EFFECTS );
        }
        else if (
            parse_conditional_structure() ||
            parse_for_loop() ||
            parse_while_loop() ||
            parse_scope( *get_current_scope_node()->find_slot( SlotFlag_CHILD ) ) )
        {}
        else
        {
            block_end_reached = true;
        }
    }

    bool is_empty = get_current_scope_node()->children().empty();
    return  is_empty ? rollback_transaction()
                     : commit_transaction();
}

Slot* Nodlang::parse_expression(u8_t _precedence, Slot* _left_override)
{
    LOG_VERBOSE("Parser", "parse expr...\n")
    LOG_VERBOSE("Parser", "%s \n", parser_state.ribbon.to_string().c_str())

    if (!parser_state.ribbon.can_eat())
    {
        LOG_VERBOSE("Parser", "parse expr..." KO " (unable to eat a single token)\n")
        return _left_override;
    }

    /*
		Get the left-handed operand
	*/
    Slot* left = _left_override;

    if( !left )
    {
                     left = parse_parenthesis_expression();
        if ( !left ) left = parse_unary_operator_expression(_precedence);
        if ( !left ) left = parse_function_call();
        if ( !left ) left = parse_variable_declaration();
        if ( !left ) left = parse_atomic_expression();
    }

    if (!parser_state.ribbon.can_eat())
    {
        LOG_VERBOSE("Parser", "parse expr... " OK " (last token reached)\n")
        return left;
    }

    /*
		Get the right-handed operand
	*/
    if ( left )
    {
        LOG_VERBOSE("Parser", "parse expr... left parsed, we parse right\n")
        Slot* expression_out = parse_binary_operator_expression(_precedence, *left);
        if ( expression_out )
        {
            if (!parser_state.ribbon.can_eat())
            {
                LOG_VERBOSE("Parser", "parse expr... " OK " right parsed (last token reached)\n")
                return expression_out;
            }
            LOG_VERBOSE("Parser", "parse expr... " OK " right parsed, recursive call...\n")
            return parse_expression(_precedence, expression_out);
        }
    }

    LOG_VERBOSE("Parser", "parse expr... left is nullptr, we return it\n")
    return left;
}

bool Nodlang::is_syntax_valid()
{
    // TODO: optimization: is this function really useful ? It check only few things.
    //                     The parsing steps that follow (parseProgram) is doing a better check, by looking to what exist in the Language.
    bool success = true;
    auto token = parser_state.ribbon.begin();
    short int opened = 0;

    while (token != parser_state.ribbon.end() && success)
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
                              parser_state.ribbon.concat_token_buffers(token->m_index, -10).c_str(),
                              token->m_string_start_pos
                          )
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
        LOG_ERROR("Parser", "Syntax Error: Bracket count mismatch, %i still opened.\n", opened)
        success = false;
    }

    return success;
}

bool Nodlang::tokenize(const std::string& _string)
{
    return tokenize(const_cast<char*>(_string.data()), _string.length());
}

bool Nodlang::tokenize(const char* buffer, size_t buffer_size)
{
    size_t global_cursor = 0;
    size_t ignored_chars_start_pos = 0;
    size_t ignored_chars_size = 0;
    while (global_cursor != buffer_size )
    {
        Token new_token = parse_token(buffer, buffer_size, global_cursor);

        if (new_token.is_null())
        {
            char buffer_portion[40];
            snprintf(buffer_portion, 40, "%s", &buffer[global_cursor]);
            LOG_WARNING("Parser", "Scanner Error: unable to tokenize \"%s...\" at index %llu\n", buffer_portion, global_cursor)
            return false;
        }

        // accumulate ignored chars (see else case to know why)
        if( new_token.m_type == Token_t::ignore)
        {
            if( ignored_chars_size == 0)
                ignored_chars_start_pos = new_token.m_string_start_pos;
            ignored_chars_size += new_token.m_string_length;
            LOG_VERBOSE("Parser", "Append \"%s\" to ignored chars\n", new_token.buffer_to_string().c_str())
        }
        else // handle ignored_chars_accumulator then push the token in the ribbon and handle ignored_chars_accumulator
        {
            // add ignored chars to the prefix or suffix of the last or new token
            // TODO: this part can be simplified
            if (ignored_chars_size != 0)
            {
                if (!parser_state.ribbon.empty())
                {
                    Token& last_token = parser_state.ribbon.back();
                     if ( allow_to_attach_suffix(last_token.m_type) )
                    {
                        last_token.m_string_length += ignored_chars_size;
                    }
                    else if (!new_token.is_null())
                    {
                        new_token.m_string_start_pos = ignored_chars_start_pos;
                        new_token.m_string_length += ignored_chars_size;
                    }
                }
                else
                {
                    parser_state.ribbon.prefix().m_string_length += ignored_chars_size;
                }
                ignored_chars_size = 0;
            }
            LOG_VERBOSE("Parser", "Push token \"%s\" to ribbon\n", new_token.buffer_to_string().c_str())
            parser_state.ribbon.push(new_token);
        }
    }

    /*
	 * Append remaining ignored_chars_accumulator to the ribbon suffix
	 */
    if (ignored_chars_size != 0)
    {
        LOG_VERBOSE("Parser", "Found ignored chars after tokenize, adding to the ribbon suffix...\n");
        Token& suffix = parser_state.ribbon.suffix();
        suffix.m_string_start_pos = ignored_chars_start_pos;
        suffix.m_string_length = ignored_chars_size;
        ignored_chars_start_pos = 0;
        ignored_chars_size = 0;
    }
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
        auto cursor = start_pos + 1;
        auto second_char = buffer[cursor];
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
    return Token::s_null;
}

Slot* Nodlang::parse_function_call()
{
    LOG_VERBOSE("Parser", "parse function call...\n")

    // Check if the minimum token count required is available ( 0: identifier, 1: open parenthesis, 2: close parenthesis)
    if (!parser_state.ribbon.can_eat(3))
    {
        LOG_VERBOSE("Parser", "parse function call... " KO " aborted, not enough tokens.\n")
        return nullptr;
    }

    start_transaction();

    // Try to parse regular function: function(...)
    std::string fct_id;
    Token token_0 = parser_state.ribbon.eat();
    Token token_1 = parser_state.ribbon.eat();
    if (token_0.m_type == Token_t::identifier &&
        token_1.m_type == Token_t::parenthesis_open)
    {
        fct_id = token_0.word_to_string();
        LOG_VERBOSE("Parser", "parse function call... " OK " regular function pattern detected.\n")
    }
    else// Try to parse operator like (ex: operator==(..,..))
    {
        Token token_2 = parser_state.ribbon.eat();// eat a "supposed open bracket>

        if (token_0.m_type == Token_t::keyword_operator && token_1.m_type == Token_t::operator_ && token_2.m_type == Token_t::parenthesis_open)
        {
            fct_id = token_1.word_to_string();// operator
            LOG_VERBOSE("Parser", "parse function call... " OK " operator function-like pattern detected.\n")
        } else
        {
            LOG_VERBOSE("Parser", "parse function call... " KO " abort, this is not a function.\n")
            rollback_transaction();
            return nullptr;
        }
    }
    std::vector<Slot*> result_slots;

    // Declare a new function prototype
    FunctionDescriptor* signature = FunctionDescriptor::create<any()>(fct_id.c_str());

    bool parsingError = false;
    while (!parsingError && parser_state.ribbon.can_eat() &&
            parser_state.ribbon.peek().m_type != Token_t::parenthesis_close)
    {
        Slot* expression_out = parse_expression();
        if ( expression_out != nullptr )
        {
            result_slots.push_back( expression_out );
            signature->push_arg( expression_out->get_property()->get_type() );
            parser_state.ribbon.eat_if(Token_t::list_separator);
        }
        else
        {
            parsingError = true;
        }
    }

    // eat "close bracket supposed" token
    if (parser_state.ribbon.eat_if(Token_t::parenthesis_close).is_null())
    {
        LOG_WARNING("Parser", "parse function call... " KO " abort, close parenthesis expected. \n")
        rollback_transaction();
        return nullptr;
    }


    // Find the prototype in the language library
    FunctionNode* fct_node = parser_state.graph->create_function(std::move(signature));

    for ( int i = 0; i < fct_node->get_arg_slots().size(); i++ )
    {
        // Connects each results to the corresponding input
        parser_state.graph->connect_or_merge( *result_slots.at(i), *fct_node->get_arg_slot(i) );
    }

    commit_transaction();
    LOG_VERBOSE("Parser", "parse function call... " OK "\n")

    return fct_node->find_slot_by_property_name( VALUE_PROPERTY, SlotFlag_OUTPUT );
}

Scope* Nodlang::get_current_scope()
{
    ASSERT( parser_state.scope.top() );
    return parser_state.scope.top();
}

Node* Nodlang::get_current_scope_node()
{
    ASSERT( parser_state.scope.top() );
    return parser_state.scope.top()->get_owner();
}

IfNode* Nodlang::parse_conditional_structure()
{
    LOG_VERBOSE("Parser", "try to parse conditional structure...\n")
    start_transaction();

    bool success = false;
    Node*   condition;
    Node*   condition_true_scope_node;
    IfNode* if_node{nullptr};
    IfNode* else_node{nullptr};

    Token if_token = parser_state.ribbon.eat_if(Token_t::keyword_if);
    if ( !if_token )
    {
        return {};
    }

    if_node = parser_state.graph->create_cond_struct();

    parser_state.graph->connect(
            *get_current_scope_node()->find_slot( SlotFlag_CHILD | SlotFlag_NOT_FULL ),
            *if_node->find_slot( SlotFlag_PARENT ),
            ConnectFlag_ALLOW_SIDE_EFFECTS );
    parser_state.scope.emplace( if_node->get_component<Scope>() );

    if_node->token_if  = parser_state.ribbon.get_eaten();

    if ( parser_state.ribbon.eat_if(Token_t::parenthesis_open) )
    {
        auto eaten_parenthesis = parser_state.ribbon.eat_if(Token_t::parenthesis_close);
        auto empty_condition = (bool)eaten_parenthesis;
        if ( !empty_condition && (condition = parse_instr()))
        {
            parser_state.graph->connect_or_merge(
                    *condition->find_slot(SlotFlag_OUTPUT),
                    if_node->condition_slot(Branch_TRUE));
        }

        if ( empty_condition || condition && parser_state.ribbon.eat_if(Token_t::parenthesis_close) )
        {
            condition_true_scope_node = parse_scope( if_node->child_slot_at(Branch_TRUE) );
            if ( condition_true_scope_node )
            {
                if ( parser_state.ribbon.eat_if(Token_t::keyword_else) )
                {
                    if_node->token_else = parser_state.ribbon.get_eaten();

                    /* parse "else" scope */
                    if ( parse_scope( if_node->child_slot_at( Branch_FALSE ) ) )
                    {
                        LOG_VERBOSE("Parser", "parse IF {...} ELSE {...} block... " OK "\n")
                        success = true;
                    }
                    /* or parse "else if" conditional structure */
                    else if ( parse_conditional_structure() )
                    {
                        LOG_VERBOSE("Parser", "parse IF {...} ELSE IF {...} block... " OK "\n")
                        success = true;
                    }
                    else
                    {
                        LOG_VERBOSE("Parser", "parse IF {...} ELSE {...} block... " KO "\n")
                    }
                } else
                {
                    LOG_VERBOSE("Parser", "parse IF {...} block... " OK "\n")
                    success = true;
                }
            }
            else
            {
                LOG_VERBOSE("Parser", "parse IF {...} block... " KO "\n")
            }
        }
        else
        {
            LOG_VERBOSE("Parser", "parse IF (...) <--- close bracket missing { ... }  " KO "\n")
            success = false;
        }
    }
    parser_state.scope.pop();

    if (success)
    {
        commit_transaction();
        return if_node;
    }

    parser_state.graph->destroy( else_node );
    parser_state.graph->destroy( condition_true_scope_node );
    parser_state.graph->destroy( condition );
    parser_state.graph->destroy( if_node );
    rollback_transaction();
    return {};
}

ForLoopNode* Nodlang::parse_for_loop()
{
    bool success = false;
    ForLoopNode* for_loop_node{};
    start_transaction();

    Token token_for = parser_state.ribbon.eat_if(Token_t::keyword_for);

    if (!token_for.is_null())
    {
        for_loop_node = parser_state.graph->create_for_loop();
        parser_state.graph->connect(
                *get_current_scope()->get_owner()->find_slot( SlotFlag_CHILD ),
                *for_loop_node->find_slot( SlotFlag_PARENT ),
                ConnectFlag_ALLOW_SIDE_EFFECTS );
        parser_state.scope.push(for_loop_node->get_component<Scope>());

        for_loop_node->token_for = token_for;

        LOG_VERBOSE("Parser", "parse FOR (...) block...\n")
        Token open_bracket = parser_state.ribbon.eat_if(Token_t::parenthesis_open);
        if (open_bracket.is_null())
        {
            LOG_ERROR("Parser", "Unable to find open bracket after for keyword.\n")
        }
        else
        {
            Node* init_instr = parse_instr();
            if (!init_instr)
            {
                LOG_ERROR("Parser", "Unable to find initial instruction.\n")
            }
            else
            {
                parser_state.graph->connect_or_merge(
                        *init_instr->find_slot( SlotFlag_OUTPUT ),
                        for_loop_node->initialization_slot() );

                Node* condition = parse_instr();
                if (!condition )
                {
                    LOG_ERROR("Parser", "Unable to find condition instruction.\n")
                }
                else
                {
                    parser_state.graph->connect_or_merge(
                            *condition->find_slot( SlotFlag_OUTPUT ),
                            for_loop_node->condition_slot(Branch_TRUE) );

                    Node* iter_instr = parse_instr();
                    if (!iter_instr)
                    {
                        LOG_ERROR("Parser", "Unable to find iterative instruction.\n")
                    }
                    else
                    {
                        parser_state.graph->connect_or_merge(
                                *iter_instr->find_slot( SlotFlag_OUTPUT ),
                                for_loop_node->iteration_slot() );

                        Token close_bracket = parser_state.ribbon.eat_if(Token_t::parenthesis_close);
                        if (close_bracket.is_null())
                        {
                            LOG_ERROR("Parser", "Unable to find close bracket after iterative instruction.\n")
                        }
                        else if (!parse_scope( for_loop_node->child_slot_at( Branch_TRUE ) ) )
                        {
                            LOG_ERROR("Parser", "Unable to parse a scope after for(...).\n")
                        }
                        else
                        {
                            success = true;
                        }
                    }
                }
            }
        }
        parser_state.scope.pop();
    }

    if (success)
    {
        commit_transaction();
    }
    else
    {
        rollback_transaction();
        parser_state.graph->destroy( for_loop_node );
        for_loop_node = {};
    }

    return for_loop_node;
}

WhileLoopNode* Nodlang::parse_while_loop()
{
    bool success = false;
    WhileLoopNode* while_loop_node = nullptr;
    start_transaction();

    Token token_while = parser_state.ribbon.eat_if(Token_t::keyword_while);

    if (!token_while.is_null())
    {
        while_loop_node = parser_state.graph->create_while_loop();
        parser_state.graph->connect(
                *get_current_scope()->get_owner()->find_slot( SlotFlag_CHILD ),
                *while_loop_node->find_slot( SlotFlag_PARENT ),
                ConnectFlag_ALLOW_SIDE_EFFECTS );
        parser_state.scope.push(while_loop_node->get_component<Scope>() );

        while_loop_node->token_while = token_while;

        LOG_VERBOSE("Parser", "parse WHILE (...) { /* block */ }\n")
        Token open_bracket = parser_state.ribbon.eat_if(Token_t::parenthesis_open);
        if (open_bracket.is_null())
        {
            LOG_ERROR("Parser", "Unable to find open bracket after \"while\"\n")
        }
        else if( Node* cond_instr = parse_instr() )
        {
            parser_state.graph->connect_or_merge(
                    *cond_instr->find_slot( SlotFlag_OUTPUT ),
                    while_loop_node->condition_slot(Branch_TRUE) );

            Token close_bracket = parser_state.ribbon.eat_if(Token_t::parenthesis_close);
            if ( close_bracket.is_null() )
            {
                LOG_ERROR("Parser", "Unable to find close bracket after condition instruction.\n")
            }
            else if (!parse_scope( while_loop_node->child_slot_at( Branch_TRUE ) ) )
            {
                LOG_ERROR("Parser", "Unable to parse a scope after \"while(\".\n")
            }
            else
            {
                success = true;
            }
        }
        parser_state.scope.pop();
    }

    if (success)
    {
        commit_transaction();
        return while_loop_node;
    }

    rollback_transaction();
    parser_state.graph->destroy( while_loop_node );
    return {};
}

Slot* Nodlang::parse_variable_declaration()
{

    if (!parser_state.ribbon.can_eat(2))
    {
        return nullptr;
    }

    start_transaction();

    Token type_token       = parser_state.ribbon.eat();
    Token identifier_token = parser_state.ribbon.eat();

    if (type_token.is_keyword_type() && identifier_token.m_type == Token_t::identifier)
    {
        const TypeDescriptor* variable_type = get_type(type_token.m_type);
        auto*           scope         = get_current_scope();
        ASSERT(scope != nullptr ) // There must always be a scope!
        VariableNode* variable_node = parser_state.graph->create_variable(variable_type, identifier_token.word_to_string(), scope );
        variable_node->set_flags(VariableFlag_DECLARED);
        variable_node->set_type_token( type_token );
        variable_node->set_identifier_token( identifier_token );
        // try to parse assignment
        Token operator_token = parser_state.ribbon.eat_if(Token_t::operator_);
        if (!operator_token.is_null() && operator_token.word_size() == 1 && *operator_token.word_ptr() == '=')
        {
            Slot* expression_out = parse_expression();
            if (expression_out != nullptr //&&
                // type::is_implicitly_convertible(expression_out->get_property()->get_type(), variable_type )
                )
            {
                parser_state.graph->connect_to_variable( *expression_out, *variable_node );
                variable_node->set_operator_token( operator_token );
            }
            else
            {
                LOG_ERROR("Parser", "Unable to parse expression to assign %s\n", identifier_token.word_to_string().c_str())
                rollback_transaction();
                parser_state.graph->destroy(variable_node );
                return nullptr;
            }
        }

        commit_transaction();
        return variable_node->find_slot(SlotFlag_OUTPUT );
    }

    rollback_transaction();
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------------
// [SECTION] C. Serializer --------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------

std::string &Nodlang::serialize_invokable(std::string &_out, const FunctionNode* _node) const
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
                    serialize_input( _out, *args[0], flags );
                }

                // Operator
                VERIFY(!_node->get_identifier_token().is_null(), "identifier token should have been assigned in parse_function_call")
                serialize_token( _out, _node->get_identifier_token() );

                // Right part of the expression
                {
                    const FunctionDescriptor* r_func_type = _node->get_connected_function_type(RIGHT_VALUE_PROPERTY);
                    bool needs_braces = r_func_type && get_precedence(r_func_type) < precedence;
                    SerializeFlags flags = SerializeFlag_RECURSE
                                         | needs_braces * SerializeFlag_WRAP_WITH_BRACES ;
                    serialize_input( _out, *args[1], flags );
                }
                break;
            }

            case 1:
            {
                // operator ( ... innerOperator ... )   ex:   -(a+b)

                ASSERT(!_node->get_identifier_token().is_null())
                serialize_token(_out, _node->get_identifier_token());

                bool needs_braces    = _node->get_connected_function_type(LEFT_VALUE_PROPERTY) != nullptr;
                SerializeFlags flags = SerializeFlag_RECURSE
                                     | needs_braces * SerializeFlag_WRAP_WITH_BRACES;
                serialize_input( _out, *args[0], flags );
                break;
            }
        }
    }
    else
    {
        serialize_func_call(_out, _node->get_func_type(), _node->get_arg_slots());
    }

    return _out;
}

std::string &Nodlang::serialize_func_call(std::string &_out, const FunctionDescriptor *_signature, const std::vector<Slot*> &inputs) const
{
    _out.append( _signature->get_identifier() );
    serialize_token_t(_out, Token_t::parenthesis_open);

    for (const Slot* input_slot : inputs)
    {
        ASSERT( input_slot->has_flags(SlotFlag_INPUT) )
        if ( input_slot != inputs.front())
        {
            serialize_token_t(_out, Token_t::list_separator);
        }
        serialize_input( _out, *input_slot, SerializeFlag_RECURSE );
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

std::string &Nodlang::serialize_token_t(std::string &_out, const Token_t &_type) const
{
    return _out.append(to_string(_type));
}

std::string &Nodlang::serialize_type(std::string &_out, const TypeDescriptor* _type) const
{
    return _out.append(to_string(_type));
}

std::string& Nodlang::serialize_variable_ref(std::string &_out, const VariableRefNode* _node) const
{
    return serialize_token( _out, _node->get_identifier_token() );
}

std::string& Nodlang::serialize_variable(std::string &_out, const VariableNode *_node) const
{
    // 1. Serialize variable's type

    if ( _node->is_instruction() )
    {
        // If parsed
        if (!_node->get_type_token().is_null())
        {
            serialize_token(_out, _node->get_type_token());
        }
        else // If created in the graph by the user
        {
            serialize_type(_out, _node->get_value()->get_type());
            _out.append(" ");
        }
    }

    // 2. Serialize variable identifier
    serialize_token( _out, _node->get_identifier_token() );

    // 3. Initialisation
    //    When a VariableNode has its input connected, we serialize it as its initialisation expression

    const Slot& slot = _node->input_slot();
    if ( _node->is_instruction() && slot.adjacent_count() != 0 )
    {
        if ( _node->get_operator_token().is_null() )
            _out.append(" = ");
        else
            _out.append(_node->get_operator_token().buffer_to_string());

        serialize_input( _out, slot, SerializeFlag_RECURSE );
    }
    return _out;
}

std::string &Nodlang::serialize_input(std::string& _out, const Slot& _slot, SerializeFlags _flags ) const
{
    ASSERT( _slot.has_flags( SlotFlag_INPUT ) );
    const Property* property      = _slot.get_property();
    const Slot*     adjacent_slot = _slot.first_adjacent();

    // In case the input slot is not connected we simply serialize the slot's related property
    if( adjacent_slot == nullptr )
        return serialize_property(_out, _slot.get_property());

    const Property* adjacent_property = adjacent_slot->get_property();
    ASSERT(adjacent_property != nullptr)

    // specific case of a Node*
    if ( adjacent_property->has_flags(PropertyFlag_IS_THIS))
        if ( Node* node = adjacent_property->get_owner() )
            return serialize_node( _out, node, _flags );

    if ( _flags & SerializeFlag_WRAP_WITH_BRACES )
        serialize_token_t(_out, Token_t::parenthesis_open);

    if (!adjacent_property->get_token().is_null())
        _out.append( adjacent_property->get_token().prefix_to_string()); // FIXME: avoid std::string copy

    // If adjacent node is a variable, we only serialize its name (no need for recursion)
    if ( adjacent_slot->get_node()->type() == NodeType_VARIABLE )
    {
        auto* variable = static_cast<const VariableNode*>(adjacent_slot->get_node());
        _out.append( variable->get_identifier_token().word_to_string() );
    }
    else if ( _flags & SerializeFlag_RECURSE && adjacent_slot )
    {
        serialize_output( _out, *adjacent_slot, SerializeFlag_RECURSE );
    }
    else
    {
        serialize_property(_out, adjacent_property );
    }

    if (!adjacent_property->get_token().is_null())
    {
        _out.append( adjacent_property->get_token().suffix_to_string()); // FIXME: avoid std::string copy
    }

    if ( _flags & SerializeFlag_WRAP_WITH_BRACES )
        serialize_token_t(_out, Token_t::parenthesis_close);

    return _out;
}

std::string &Nodlang::serialize_output(std::string& _out, const Slot& _slot, SerializeFlags _flags) const
{
    /** This method is work in progress
     * It works only if the given slot is an output of a VALUE_PROPERTY.
     * This means we need to serialize the node itself */
    ASSERT( _slot.has_flags( SlotFlag_OUTPUT ) )
    ASSERT( _slot.get_property() == _slot.get_node()->get_prop(VALUE_PROPERTY) )
    return serialize_node(_out, _slot.get_node(), _flags );
}

std::string & Nodlang::serialize_node(std::string &_out, const Node* node, SerializeFlags _flags ) const
{
    ASSERT( node )
    ASSERT( _flags == SerializeFlag_RECURSE ); // The only flag configuration handled for now

    switch ( node->type() )
    {
        case NodeType_BLOCK_CONDITION:
            serialize_cond_struct(_out, static_cast<const IfNode*>(node) );
            break;
        case NodeType_BLOCK_FOR_LOOP:
            serialize_for_loop(_out, static_cast<const ForLoopNode*>(node) );
            break;
        case NodeType_BLOCK_WHILE_LOOP:
            serialize_while_loop(_out, static_cast<const WhileLoopNode*>(node) );
            break;
        case NodeType_LITERAL:
            serialize_property(_out, static_cast<const LiteralNode*>(node)->value());
            break;
        case NodeType_VARIABLE:
            serialize_variable(_out, static_cast<const VariableNode*>(node));
            break;
        case NodeType_VARIABLE_REF:
            serialize_variable_ref(_out, static_cast<const VariableRefNode*>(node));
            break;
        case NodeType_BLOCK_SCOPE:
            serialize_scope(_out, node->get_component<Scope>() );
            break;
        case NodeType_FUNCTION:
            [[fallthrough]];
        case NodeType_OPERATOR:
            serialize_invokable(_out, static_cast<const FunctionNode*>(node) );
            break;
        default:
            VERIFY(false, "Unhandled NodeType, can't serialize");
    }

    return serialize_token(_out, node->get_suffix() );
}

std::string &Nodlang::serialize_scope(std::string &_out, const Scope *_scope) const
{
    serialize_token(_out, _scope->token_begin);
    for (const Node* child : _scope->get_owner()->children() )
    {
        serialize_node( _out, child, SerializeFlag_RECURSE );
    }
    return serialize_token(_out, _scope->token_end);
}

std::string &Nodlang::serialize_token(std::string& _out, const Token& _token) const
{
    if (!_token.is_null() && _token.has_buffer())
    {
        _out.append(_token.string_ptr(), _token.string_size());
    }
    return _out;
}

std::string& Nodlang::serialize_bool(std::string& _out, bool b) const
{
    _out.append( b ? "true" : "false");
    return _out;
}

std::string& Nodlang::serialize_int(std::string& _out, int i) const
{
    _out.append( std::to_string(i) );
    return _out;
}

std::string& Nodlang::serialize_double(std::string& _out, double d) const
{
    _out.append( std::to_string(d) );
    return _out;
}

std::string &Nodlang::serialize_for_loop(std::string &_out, const ForLoopNode *_for_loop) const
{
    if( _for_loop->token_for.is_null())
    {
        serialize_token_t(_out, Token_t::keyword_for);
    }
    else
    {
        serialize_token(_out, _for_loop->token_for);
    }

    serialize_token_t(_out, Token_t::parenthesis_open);

    const Slot& init_slot = *_for_loop->find_slot_by_property_name( INITIALIZATION_PROPERTY, SlotFlag_INPUT );
    const Slot& cond_slot = *_for_loop->find_slot_by_property_name( CONDITION_PROPERTY, SlotFlag_INPUT );
    const Slot& iter_slot = *_for_loop->find_slot_by_property_name( ITERATION_PROPERTY, SlotFlag_INPUT );

    serialize_input( _out, init_slot, SerializeFlag_RECURSE );
    serialize_input( _out, cond_slot, SerializeFlag_RECURSE );
    serialize_input( _out, iter_slot, SerializeFlag_RECURSE );

    serialize_token_t(_out, Token_t::parenthesis_close);

    // if scope
    if ( auto scope = _for_loop->scope_at( Branch_TRUE ) )
    {
        serialize_scope(_out, scope );
    }
    else
    {
        // When created manually, no scope is created, we serialize a fake one
        _out.append("\n{\n}\n");
    }

    return _out;
}

std::string &Nodlang::serialize_while_loop(std::string &_out, const WhileLoopNode *_while_loop_node) const
{

    if( _while_loop_node->token_while.is_null())
    {
        serialize_token_t(_out, Token_t::keyword_while);
    }
    else
    {
        serialize_token(_out, _while_loop_node->token_while);
    }

    serialize_token_t(_out, Token_t::parenthesis_open);

    if( Node* condition = _while_loop_node->condition(Branch_TRUE) )
    {
        serialize_node(_out, condition, SerializeFlag_RECURSE);
    }

    serialize_token_t(_out, Token_t::parenthesis_close);

    // if scope
    if ( auto scope = _while_loop_node->scope_at( Branch_TRUE ) )
    {
        serialize_scope(_out, scope );
    }
    else
    {
        // When created manually, no scope is created, we serialize a fake one
        _out.append("\n{\n}\n");
    }

    return _out;
}


std::string &Nodlang::serialize_cond_struct(std::string &_out, const IfNode*_condition_struct ) const
{
    // if ...
    if ( _condition_struct->token_if.is_null())
    {
        serialize_token_t(_out, Token_t::keyword_if);
    }
    else
    {
        serialize_token(_out, _condition_struct->token_if);
    }

    // ... ( <condition> )
    serialize_token_t(_out, Token_t::parenthesis_open);
    if ( Node* condition = _condition_struct->condition(Branch_TRUE) )
    {
        serialize_node(_out, condition, SerializeFlag_RECURSE);
    }
    serialize_token_t(_out, Token_t::parenthesis_close);

    // ... ( ... ) <scope>
    if ( Scope* scope = _condition_struct->scope_at( Branch_TRUE ) )
    {
        serialize_scope(_out, scope );
    }
    else
    {
        // When scope is undefined, serialized a fake one
        serialize_token_t(_out, Token_t::end_of_line);
        serialize_token_t(_out, Token_t::scope_begin);
        serialize_token_t(_out, Token_t::end_of_line);
        serialize_token_t(_out, Token_t::scope_end);
    }

    // else & else scope
    if ( _condition_struct->token_else )
    {
        serialize_token(_out, _condition_struct->token_else);
        if ( const Scope* else_scope = _condition_struct->scope_at( Branch_FALSE ) )
        {
            serialize_node( _out, else_scope->get_owner(), SerializeFlag_RECURSE );
        }
    }
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
    return serialize_token(_out, _property->get_token());
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

std::string &Nodlang::to_string(std::string &_out, const TypeDescriptor* _type) const
{
    auto found = m_keyword_by_type_id.find(_type->id());
    if (found != m_keyword_by_type_id.cend())
    {
        return _out.append(found->second);
    }
    return _out;
}

std::string &Nodlang::to_string(std::string &_out, Token_t _token_t) const
{
    switch (_token_t)
    {
        case Token_t::end_of_line:     return _out.append("\n"); // TODO: handle all platforms
        case Token_t::ignore:          return _out.append("ignore");
        case Token_t::operator_:       return _out.append("operator");
        case Token_t::identifier:      return _out.append("identifier");
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

std::string Nodlang::to_string(const TypeDescriptor* _type) const
{
    std::string result;
    return to_string(result, _type);
}

std::string Nodlang::to_string(Token_t _token) const
{
    std::string result;
    return to_string(result, _token);
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
    VERIFY(is_a_type_keyword(_token), "_token_t is not a type keyword!");
    return m_type_by_token_t.find(_token)->second;
}

Token Nodlang::parse_token(const std::string &_string) const
{
    size_t cursor = 0;
    return parse_token( const_cast<char*>(_string.data()), _string.length(), cursor);
}

bool Nodlang::allow_to_attach_suffix(Token_t type) const
{
    return    type != Token_t::identifier          // identifiers must stay clean because they are reused
              && type != Token_t::parenthesis_open    // ")" are lost when creating AST
              && type != Token_t::parenthesis_close;  // "(" are lost when creating AST
}

Nodlang::ParserState::ParserState()
: graph(nullptr)
, source_buffer(nullptr)
, source_buffer_size(0)
{}

Nodlang::ParserState::~ParserState()
{
    // delete[] source_buffer; NOT owned!
}

void Nodlang::ParserState::set_source_buffer(const char *str, size_t size)
{
    ASSERT(source_buffer == nullptr); // should call clear() before
    ASSERT(str != nullptr);

    source_buffer      = str;
    source_buffer_size = size;
    ribbon.set_source_buffer(source_buffer);
}

void Nodlang::ParserState::clear()
{
    graph = nullptr;
    ribbon.clear();
    source_buffer      = nullptr;
    source_buffer_size = 0;
    while(!scope.empty())
    {
        scope.pop();
    }
}

Nodlang* ndbl::init_language()
{
    ASSERT(g_language == nullptr)
    g_language = new Nodlang();
    return g_language;
}

Nodlang* ndbl::get_language()
{
    return g_language;
}

void ndbl::shutdown_language(Nodlang* _language)
{
    ASSERT(g_language == _language) // singleton for now
    ASSERT(g_language != nullptr)
    delete g_language;
    g_language = nullptr;
}