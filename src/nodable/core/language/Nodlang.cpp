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

#include "fw/core/reflection/reflection"
#include "fw/core/format.h"
#include "fw/core/log.h"
#include "fw/core/hash.h"

#include "core/ConditionalStructNode.h"
#include "core/DirectedEdge.h"
#include "core/ForLoopNode.h"
#include "core/Graph.h"
#include "core/InstructionNode.h"
#include "core/InvokableComponent.h"
#include "core/LiteralNode.h"
#include "core/Pool.h"
#include "core/Property.h"
#include "core/Scope.h"
#include "core/VariableNode.h"
#include "core/WhileLoopNode.h"
#include "core/language/Nodlang_biology.h"
#include "core/language/Nodlang_math.h"

using namespace ndbl;
using namespace fw;

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
         { "bool",   Token_t::keyword_bool,    type::get<bool>()},
         { "string", Token_t::keyword_string,  type::get<std::string>()},
         { "double", Token_t::keyword_double,  type::get<double>()},
         { "i16",    Token_t::keyword_i16,     type::get<i16_t>()},
         { "int",    Token_t::keyword_int,     type::get<i32_t>()}
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
        m_token_t_by_keyword.insert({hash::hash(keyword), token_t});
        m_keyword_by_token_t.insert({token_t, keyword});
    }

    for( auto [keyword, token_t, type] : m_definition.types)
    {
        m_keyword_by_token_t.insert({token_t, keyword});
        m_keyword_by_type_id.insert({type->id(), keyword});
        m_token_t_by_keyword.insert({hash::hash(keyword), token_t});
        m_token_t_by_type_id.insert({type->id(), token_t});
        m_type_by_token_t.insert({token_t, type});
    }

    for( auto [keyword, operator_t, precedence] : m_definition.operators)
    {
        const Operator *op = new Operator(keyword, operator_t, precedence);
        FW_ASSERT(std::find(m_operators.begin(), m_operators.end(), op) == m_operators.end())
        m_operators.push_back(op);
    }

    // A.3. Load libraries
    //---------------------

    load_library<Nodlang_math>();     // contains all operator implementations
    load_library<Nodlang_biology>();  // a function to convert RNA (library is wip)
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

    PoolID<Node> program = parse_program();

    high_resolution_clock::time_point parse_program_end = high_resolution_clock::now();
    LOG_MESSAGE("Parser", "%16s == %.3f ms\n", "parse_program()", duration_cast<duration<double>>(parse_program_end - tokenize_end).count() * 1000.0)

    if ( program.get() == nullptr )
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
        eachNode->dirty = false;
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
    FW_ASSERT(_quoted_str.size() >= 2);
    FW_ASSERT(_quoted_str.front() == '\"');
    FW_ASSERT(_quoted_str.back() == '\"');
    return std::string(++_quoted_str.cbegin(), --_quoted_str.cend());
}

double Nodlang::to_double(const std::string &_str)
{
    return stod(_str);
}

Slot *Nodlang::parse_token(Token _token)
{
    if (_token.m_type == Token_t::identifier)
    {
        PoolID<VariableNode> variable = get_current_scope()->find_variable(_token.word_to_string() );

        if ( variable.get() == nullptr)
        {
            if (m_strict_mode)
            {
                LOG_ERROR("Parser", "Expecting declaration for symbol %s (strict mode) \n", _token.word_to_string().c_str())
            }
            else
            {
                /* when strict mode is OFF, we just create a variable with Any type */
                LOG_WARNING("Parser", "Expecting declaration for symbol %s, compilation will fail.\n",
                            _token.word_to_string().c_str())
                variable = parser_state.graph->create_variable(type::null(), _token.word_to_string(), get_current_scope() );
                variable->property()->token = std::move(_token);
                variable->set_declared(false);
            }
        }
        else
        {
            FW_ASSERT(variable->is_declared());
        }

        if ( variable.get() != nullptr )
        {
            return variable->find_value_typed_slot( SlotFlag_OUTPUT );
        }
        return nullptr;
    }

    PoolID<LiteralNode> literal;

    switch (_token.m_type)
    {
        case Token_t::literal_bool:
        {
            literal = parser_state.graph->create_literal<bool>();
            literal->value()->set(to_bool(_token.word_to_string())); // FIXME: avoid std::string copy
            break;
        }

        case Token_t::literal_int:
        {
            literal = parser_state.graph->create_literal<i32_t>();
            i32_t value = stoi((_token.word_to_string())); // FIXME: avoid std::string copy
            literal->value()->set( value );
            break;
        }

        case Token_t::literal_double:
        {
            literal = parser_state.graph->create_literal<double>();
            literal->value()->set(to_double(_token.word_to_string())); // FIXME: avoid std::string copy
            break;
        }

        case Token_t::literal_string:
        {
            literal = parser_state.graph->create_literal<std::string>();
            literal->value()->set(to_unquoted_string(_token.word_to_string()));
            break;
        }

        default:;
    }

    if (literal)
    {
        Slot* output_slot = literal->find_slot( VALUE_PROPERTY, SlotFlag_OUTPUT );
        output_slot->get_property()->token = std::move(_token);
        return output_slot;
    }

    LOG_VERBOSE("Parser", "Unable to perform token_to_property for token %s!\n", _token.word_to_string().c_str())
    return nullptr;
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
    func_type *type = new func_type(ope->identifier);
    type->set_return_type(type::any());
    type->push_args(
            _left.get_property()->get_type(),
            right->get_property()->get_type());

    PoolID<InvokableComponent> component;
    PoolID<Node> binary_op;

    if (auto invokable = find_operator_fct(type))
    {
        // concrete operator
        binary_op = parser_state.graph->create_operator(invokable.get());
        component = binary_op->get_component<InvokableComponent>();
        delete type;
    }
    else if (type)
    {
        // abstract operator
        binary_op = parser_state.graph->create_abstract_operator(type);
        component = binary_op->get_component<InvokableComponent>();
    }
    else
    {
        LOG_VERBOSE("Parser", "parse binary operation expr... " KO " no signature\n")
        rollback_transaction();
        return nullptr;
    }

    component->token = operator_token;
    parser_state.graph->connect_or_merge( &_left, binary_op->find_slot( LEFT_VALUE_PROPERTY, SlotFlag_INPUT ) );
    parser_state.graph->connect_or_merge( right, binary_op->find_slot( RIGHT_VALUE_PROPERTY, SlotFlag_INPUT ) );

    commit_transaction();
    LOG_VERBOSE("Parser", "parse binary operation expr... " OK "\n")
    return binary_op->find_slot( VALUE_PROPERTY, SlotFlag_OUTPUT );
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
    auto* type = new func_type(operator_token.word_to_string());  // FIXME: avoid std::string copy
    type->set_return_type(type::any());
    type->push_args( out_atomic->get_property()->get_type());

    PoolID<InvokableComponent> component;
    PoolID<Node> node;

    if (auto invokable = find_operator_fct(type))
    {
        node = parser_state.graph->create_operator(invokable.get());
        delete type;
    }
    else
    {
        node = parser_state.graph->create_abstract_operator(type);
    }

    component = node->get_component<InvokableComponent>();
    component->token = std::move( operator_token );

    parser_state.graph->connect_or_merge( out_atomic, node->find_slot( LEFT_VALUE_PROPERTY, SlotFlag_INPUT ) );

    LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " OK "\n")
    commit_transaction();

    return node->find_slot( VALUE_PROPERTY, SlotFlag_OUTPUT );
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

PoolID<InstructionNode> Nodlang::parse_instr()
{
    start_transaction();

    Slot* expression_out = parse_expression();

    if ( expression_out == nullptr)
    {
        LOG_VERBOSE("Parser", "parse instruction " KO " (parsed is nullptr)\n")
        rollback_transaction();
        return PoolID<InstructionNode>::null;
    }

    PoolID<InstructionNode> instr_node = parser_state.graph->create_instr();

    if (parser_state.ribbon.can_eat())
    {
        Token expected_end_of_instr_token = parser_state.ribbon.eat_if(Token_t::end_of_instruction);
        if (!expected_end_of_instr_token.is_null())
        {
            instr_node->token_end = expected_end_of_instr_token;
        }
        else if (parser_state.ribbon.peek().m_type != Token_t::parenthesis_close)
        {
            LOG_VERBOSE("Parser", "parse instruction " KO " (end of instruction not found)\n")
            rollback_transaction();
            return PoolID<InstructionNode>::null;
        }
    }

    parser_state.graph->connect_to_instruction( expression_out, instr_node.get() );

    LOG_VERBOSE("Parser", "parse instruction " OK "\n")
    commit_transaction();
    return instr_node->poolid();
}

PoolID<Node> Nodlang::parse_program()
{
    start_transaction();

    parser_state.graph->clear();
    PoolID<Node> root = parser_state.graph->create_root();
    PoolID<Scope> program_scope = root->get_component<Scope>();
    parser_state.scope.emplace(program_scope);

    parse_code_block(program_scope);// we do not check if we parsed something empty or not, a program can be empty.

    // Add ignored chars pre/post token to the main scope begin/end token prefix/suffix.
    FW_ASSERT(program_scope->token_begin.is_null())
    FW_ASSERT(program_scope->token_end.is_null())
    program_scope->token_begin = parser_state.ribbon.prefix();
    program_scope->token_end = parser_state.ribbon.suffix();

    parser_state.scope.pop();
    commit_transaction();

    return root;
}

PoolID<Node> Nodlang::parse_scope()
{
    PoolID<Node> result;

    start_transaction();

    if (parser_state.ribbon.eat_if(Token_t::scope_begin).is_null())
    {
        rollback_transaction();
    }
    else
    {
        PoolID<Node>  scope_node = parser_state.graph->create_scope();
        PoolID<Scope> scope      = scope_node->get_component<Scope>();
        /*
         * link scope with parent_scope.
         * They must be linked in order to find_variables recursively.
         */
        Scope* parent_scope = get_current_scope().get();
        if (parent_scope)
        {
            PoolID<Node> parent_scope_node = parent_scope->get_owner();
            parser_state.graph->connect(
                    scope_node->find_slot( THIS_PROPERTY, SlotFlag_PARENT ),
                    parent_scope_node->find_slot( THIS_PROPERTY, SlotFlag_CHILD ),
                    SideEffects::ON );
        }

        parser_state.scope.emplace(scope);

        scope->token_begin = parser_state.ribbon.get_eaten();

        parse_code_block( get_current_scope() );

        if (parser_state.ribbon.eat_if(Token_t::scope_end).is_null())
        {
            parser_state.graph->destroy( scope_node );
            rollback_transaction();
            result.reset();
        }
        else
        {
            scope->token_end = parser_state.ribbon.get_eaten();
            commit_transaction();
            result = scope_node->poolid();
        }

        parser_state.scope.pop();
    }
    return result;
}

PoolID<Scope> Nodlang::parse_code_block(PoolID<Scope> curr_scope)
{
    FW_ASSERT(curr_scope);

    start_transaction();

    while (parser_state.ribbon.can_eat())
    {
        if ( InstructionNode* instr_node = parse_instr().get() )
        {
            // Create parent/child connection
            Slot* out = get_current_scope_node()->find_slot( THIS_PROPERTY, SlotFlag_CHILD );
            Slot* in  = instr_node->find_slot( THIS_PROPERTY, SlotFlag_PARENT );
            parser_state.graph->connect( out, in, SideEffects::ON );
            continue;
        }

        if (
            parse_conditional_structure() ||
            parse_for_loop() ||
            parse_while_loop() ||
            parse_scope()
        )
        {
            continue;
        }

        break;
    }

    if ( curr_scope->get_owner()->children().empty() )
    {
        rollback_transaction();
        return PoolID<Scope>::null;
    }
    else
    {
        commit_transaction();
        return curr_scope;
    }
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
                              token->m_buffer_start_pos
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

bool Nodlang::tokenize(char* buffer, size_t buffer_size)
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
                ignored_chars_start_pos = new_token.m_buffer_start_pos;
            ignored_chars_size += new_token.m_buffer_size;
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
                        last_token.m_buffer_size += ignored_chars_size;
                    }
                    else if (!new_token.is_null())
                    {
                        new_token.m_buffer_start_pos = ignored_chars_start_pos;
                        new_token.m_buffer_size += ignored_chars_size;
                    }
                }
                else
                {
                    parser_state.ribbon.prefix().m_buffer_size += ignored_chars_size;
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
        suffix.m_buffer_start_pos = ignored_chars_start_pos;
        suffix.m_buffer_size = ignored_chars_size;
        ignored_chars_start_pos = 0;
        ignored_chars_size = 0;
    }
    return true;
}

Token Nodlang::parse_token(char* buffer, size_t buffer_size, size_t& global_cursor) const
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
            return Token{Token_t::ignore, buffer, start_pos, cursor - start_pos};
        }
    }

    // single-char
    auto single_char_found = m_token_t_by_single_char.find(first_char);
    if( single_char_found != m_token_t_by_single_char.end() )
    {
        ++global_cursor;
        const Token_t type = single_char_found->second;
        return Token{type, buffer, start_pos, 1};
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
                return Token{Token_t::operator_, buffer, start_pos, cursor - start_pos};
            }
            // "="
            global_cursor++;
            return Token{Token_t::operator_, buffer, start_pos, 1};
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
            return Token{Token_t::operator_, buffer, start_pos, cursor - start_pos};
        }
    }

    // number (double)
    //     note: we accept zeros as prefix (ex: "0002.15454", or "01012")
    if ( is_digit(first_char) )
    {
        auto cursor = start_pos + 1;
        Token_t type = Token_t::literal_int;

        // integer
        while (cursor != buffer_size && is_digit(buffer[cursor]))
        {
            ++cursor;
        }

        // double
        if(cursor + 1 < buffer_size
           && buffer[cursor] == '.'      // has a decimal separator
            && is_digit(buffer[cursor + 1]) // followed by a digit
           )
        {
            auto local_cursor_decimal_separator = cursor;
            ++cursor;

            // decimal portion
            while (cursor != buffer_size && is_digit(buffer[cursor]))
            {
                ++cursor;
            }
            type = Token_t::literal_double;
        }
        global_cursor = cursor;
        return Token{type, buffer, start_pos, cursor - start_pos};
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
        return Token{Token_t::literal_string, buffer, start_pos, cursor - start_pos};
    }

    // symbol (identifier or keyword)
    if (is_letter(first_char) || first_char == '_' )
    {
        // parse symbol
        auto cursor = start_pos + 1;
        while (cursor != buffer_size && is_letter(buffer[cursor]) || is_digit(buffer[cursor]) || buffer[cursor] == '_' )
        {
            ++cursor;
        }
        global_cursor = cursor;

        Token_t type = Token_t::identifier;

        auto keyword_found = m_token_t_by_keyword.find( hash::hash(buffer + start_pos, cursor - start_pos) );
        if (keyword_found != m_token_t_by_keyword.end())
        {
            // a keyword has priority over identifier
            type = keyword_found->second;
        }
        return Token{type, buffer, start_pos, cursor - start_pos};
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
    } else// Try to parse operator like (ex: operator==(..,..))
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
    func_type signature(fct_id);
    signature.set_return_type(type::any());

    bool parsingError = false;
    while (!parsingError && parser_state.ribbon.can_eat() &&
            parser_state.ribbon.peek().m_type != Token_t::parenthesis_close)
    {
        Slot* expression_out = parse_expression();
        if ( expression_out != nullptr )
        {
            result_slots.push_back( expression_out );
            signature.push_arg( expression_out->get_property()->get_type() );
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
    std::shared_ptr<const iinvokable> invokable = find_function(&signature);

    PoolID<Node> fct_node_id;
    if (invokable)
    {
        /*
         * If we found a function matching signature, we create a node with that function.
         * The node will be able to be evaluated.
         *
         * TODO: remove this method, the parser should not check if function exist or not.
         *       this role is for the Compiler.
         */
        fct_node_id = parser_state.graph->create_function(invokable.get());
    }
    else
    {
        /*
         * If we DO NOT found a function matching signature, we create an abstract function.
         * The node will NOT be able to be evaluated.
         */
        fct_node_id = parser_state.graph->create_abstract_function(&signature);
    }

    Node* fct_node{ fct_node_id.get() };
    for ( FuncArg& signature_arg : signature.get_args() )
    {
        // Connects each results to the corresponding input
        auto& out_slot = result_slots.at(signature_arg.m_index);
        Slot* in_slot  = fct_node->find_slot( signature_arg.m_name.c_str(), SlotFlag_INPUT );
        parser_state.graph->connect_or_merge( out_slot, in_slot );
    }

    commit_transaction();
    LOG_VERBOSE("Parser", "parse function call... " OK "\n")

    return fct_node->find_slot( VALUE_PROPERTY, SlotFlag_OUTPUT );
}

PoolID<Scope> Nodlang::get_current_scope()
{
    FW_ASSERT( parser_state.scope.top() );
    return parser_state.scope.top();
}

PoolID<Node> Nodlang::get_current_scope_node()
{
    FW_ASSERT( parser_state.scope.top() );
    return parser_state.scope.top()->get_owner();
}

PoolID<ConditionalStructNode> Nodlang::parse_conditional_structure()
{
    LOG_VERBOSE("Parser", "try to parse conditional structure...\n")
    start_transaction();

    bool success = false;
    PoolID<InstructionNode>       condition;
    PoolID<Node>                  scopeIf;
    PoolID<ConditionalStructNode> conditional_struct_node;
    PoolID<ConditionalStructNode> else_cond_struct;

    Token if_token = parser_state.ribbon.eat_if(Token_t::keyword_if);
    if ( if_token.is_null() )
    {
        return PoolID<ConditionalStructNode>::null;
    }

    conditional_struct_node = parser_state.graph->create_cond_struct();
    Node* scope_node = get_current_scope()->get_owner().get();

    parser_state.graph->connect(
            conditional_struct_node->find_slot( THIS_PROPERTY, SlotFlag_PARENT ),
            scope_node->find_slot( THIS_PROPERTY, SlotFlag_CHILD ),
            SideEffects::ON );
    parser_state.scope.push(conditional_struct_node->get_component<Scope>()->poolid());

    conditional_struct_node->token_if  = parser_state.ribbon.get_eaten();

    if (!parser_state.ribbon.eat_if(Token_t::parenthesis_open).is_null())
    {
        bool empty_parenthesis = !parser_state.ribbon.eat_if(Token_t::parenthesis_close).is_null();
        if (!empty_parenthesis && (condition = parse_instr()))
        {
            condition->set_name("Condition");
            condition->set_name("Cond.");
            conditional_struct_node->cond_expr = condition;
            parser_state.graph->connect_or_merge(
                    condition->find_slot( THIS_PROPERTY, SlotFlag_OUTPUT ),
                    conditional_struct_node->find_slot( CONDITION_PROPERTY, SlotFlag_INPUT ) );
        }

        if ( empty_parenthesis || ( condition.get() && !parser_state.ribbon.eat_if(Token_t::parenthesis_close).is_null()))
        {
            scopeIf = parse_scope();
            if ( scopeIf.get() != nullptr )
            {
                if (!parser_state.ribbon.eat_if(Token_t::keyword_else).is_null())
                {
                    conditional_struct_node->token_else = parser_state.ribbon.get_eaten();

                    /* parse else scope */
                    if ( parse_scope().get() )
                    {
                        LOG_VERBOSE("Parser", "parse IF {...} ELSE {...} block... " OK "\n")
                        success = true;
                    }
                        /* (or) parse else if scope */
                    else if ( parse_conditional_structure().get() )
                    {
                        LOG_VERBOSE("Parser", "parse IF {...} ELSE IF {...} block... " OK "\n")
                        success = true;
                    } else
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
        return conditional_struct_node;
    }

    parser_state.graph->destroy( else_cond_struct );
    parser_state.graph->destroy( scopeIf );
    parser_state.graph->destroy( condition );
    parser_state.graph->destroy(conditional_struct_node);
    rollback_transaction();
    return PoolID<ConditionalStructNode>::null;
}

PoolID<ForLoopNode> Nodlang::parse_for_loop()
{
    bool success = false;
    PoolID<ForLoopNode> for_loop_node;
    start_transaction();

    Token token_for = parser_state.ribbon.eat_if(Token_t::keyword_for);

    if (!token_for.is_null())
    {
        for_loop_node = parser_state.graph->create_for_loop();
        parser_state.graph->connect(
                for_loop_node->find_slot( THIS_PROPERTY, SlotFlag_PARENT ),
                get_current_scope()->get_owner()->find_slot( THIS_PROPERTY, SlotFlag_CHILD ),
                SideEffects::ON );
        parser_state.scope.push(for_loop_node->get_component<Scope>()->poolid());

        for_loop_node->token_for = token_for;

        LOG_VERBOSE("Parser", "parse FOR (...) block...\n")
        Token open_bracket = parser_state.ribbon.eat_if(Token_t::parenthesis_open);
        if (open_bracket.is_null())
        {
            LOG_ERROR("Parser", "Unable to find open bracket after for keyword.\n")
        } else
        {
            PoolID<InstructionNode> init_instr = parse_instr();
            if (!init_instr.get())
            {
                LOG_ERROR("Parser", "Unable to find initial instruction.\n")
            } else
            {
                init_instr->set_name("Initialisation");
                parser_state.graph->connect_or_merge(
                        init_instr->find_slot( THIS_PROPERTY, SlotFlag_OUTPUT ),
                        for_loop_node->find_slot( INITIALIZATION_PROPERTY, SlotFlag_INPUT ) );
                for_loop_node->init_instr = init_instr;

                PoolID<InstructionNode> cond_instr = parse_instr();
                if (!cond_instr.get())
                {
                    LOG_ERROR("Parser", "Unable to find condition instruction.\n")
                } else
                {
                    cond_instr->set_name("Condition");
                    parser_state.graph->connect_or_merge(
                            cond_instr->find_slot( THIS_PROPERTY, SlotFlag_OUTPUT ),
                            for_loop_node->find_slot( CONDITION_PROPERTY, SlotFlag_INPUT ) );

                    for_loop_node->cond_instr = cond_instr;

                    PoolID<InstructionNode> iter_instr = parse_instr();
                    if (!iter_instr.get())
                    {
                        LOG_ERROR("Parser", "Unable to find iterative instruction.\n")
                    }
                    else
                    {
                        iter_instr->set_name("Iteration");
                        parser_state.graph->connect_or_merge(
                                iter_instr->find_slot( THIS_PROPERTY, SlotFlag_OUTPUT ),
                                for_loop_node->find_slot( ITERATION_PROPERTY, SlotFlag_INPUT ) );
                        for_loop_node->iter_instr = iter_instr;

                        Token close_bracket = parser_state.ribbon.eat_if(Token_t::parenthesis_close);
                        if (close_bracket.is_null())
                        {
                            LOG_ERROR("Parser", "Unable to find close bracket after iterative instruction.\n")
                        } else if (!parse_scope().get())
                        {
                            LOG_ERROR("Parser", "Unable to parse a scope after for(...).\n")
                        } else
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
    } else
    {
        rollback_transaction();
        parser_state.graph->destroy( for_loop_node );
        for_loop_node.reset();
    }

    return for_loop_node;
}

PoolID<WhileLoopNode> Nodlang::parse_while_loop()
{
    bool success = false;
    PoolID<WhileLoopNode> while_loop_node;
    start_transaction();

    Token token_while = parser_state.ribbon.eat_if(Token_t::keyword_while);

    if (!token_while.is_null())
    {
        while_loop_node = parser_state.graph->create_while_loop();
        parser_state.graph->connect(
                while_loop_node->find_slot( THIS_PROPERTY, SlotFlag_PARENT ),
                get_current_scope()->get_owner()->find_slot( THIS_PROPERTY, SlotFlag_CHILD ),
                SideEffects::ON );
        parser_state.scope.push(while_loop_node->get_component<Scope>() );

        while_loop_node->token_while = token_while;

        LOG_VERBOSE("Parser", "parse WHILE (...) { /* block */ }\n")
        Token open_bracket = parser_state.ribbon.eat_if(Token_t::parenthesis_open);
        if (open_bracket.is_null())
        {
            LOG_ERROR("Parser", "Unable to find open bracket after \"while\"\n")
        }
        else if( InstructionNode* cond_instr = parse_instr().get())
        {
            cond_instr->set_name("Condition");
            while_loop_node->cond_instr = cond_instr->poolid();
            parser_state.graph->connect_or_merge(
                    cond_instr->find_slot( THIS_PROPERTY, SlotFlag_OUTPUT ),
                    while_loop_node->find_slot( CONDITION_PROPERTY, SlotFlag_INPUT ) );

            Token close_bracket = parser_state.ribbon.eat_if(Token_t::parenthesis_close);
            if (close_bracket.is_null())
            {
                LOG_ERROR("Parser", "Unable to find close bracket after condition instruction.\n")
            }
            else if (!parse_scope().get())
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
    return PoolID<WhileLoopNode>::null;
}

Slot* Nodlang::parse_variable_declaration()
{

    if (!parser_state.ribbon.can_eat(2))
    {
        return nullptr;
    }

    start_transaction();

    Token type_token = parser_state.ribbon.eat();
    Token identifier_token = parser_state.ribbon.eat();

    if (type_token.is_keyword_type() && identifier_token.m_type == Token_t::identifier)
    {
        const type* variable_type = get_type(type_token.m_type);
        PoolID<VariableNode> variable_id = parser_state.graph->create_variable( variable_type, identifier_token.word_to_string(), get_current_scope() );
        {
            VariableNode* variable = variable_id.get(); // dereference once to apply several changes
            variable->set_declared( true );
            variable->type_token = type_token;
            variable->identifier_token.move_prefixsuffix( &identifier_token );
            variable->property()->token = identifier_token;
        }
        // try to parse assignment
        Token operator_token = parser_state.ribbon.eat_if(Token_t::operator_);
        if (!operator_token.is_null() && operator_token.word_size() == 1 && *operator_token.word() == '=')
        {
            Slot* expression_out = parse_expression();
            if (expression_out != nullptr &&
                type::is_implicitly_convertible(expression_out->get_property()->get_type(), variable_type ) )
            {
                parser_state.graph->connect_to_variable( expression_out, variable_id );
                variable_id->assignment_operator_token = operator_token;
            }
            else
            {
                LOG_ERROR("Parser", "Unable to parse expression to assign %s\n", identifier_token.word_to_string().c_str())
                rollback_transaction();
                parser_state.graph->destroy( variable_id );
                return nullptr;
            }
        }

        commit_transaction();
        return variable_id->find_slot( THIS_PROPERTY, SlotFlag_OUTPUT );
    }

    rollback_transaction();
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------------
// [SECTION] C. Serializer --------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------

std::string &Nodlang::serialize_invokable(std::string &_out, const InvokableComponent *_component) const
{
    const func_type* type  = _component->get_func_type();
    const Node*      owner = _component->get_owner().get();

    if (_component->is_operator())
    {
        // generic serialize property lambda
        auto serialize_property_with_or_without_brackets = [this, &_out](const Slot* slot, bool needs_brackets)
        {
            if (!needs_brackets)
            {
                serialize_edge(_out, slot );
                return;
            }
            serialize_token_t(_out, Token_t::parenthesis_open);
            serialize_edge(_out, slot );
            serialize_token_t(_out, Token_t::parenthesis_close);
        };

        const std::vector<SlotRef>& args = _component->get_arguments();
        int precedence = get_precedence(_component->get_function());

        switch (type->get_arg_count())
        {
            case 2:
            {
                // Left part of the expression
                {
                    auto l_handed_invokable = owner->get_connected_invokable(LEFT_VALUE_PROPERTY);
                    bool needs_brackets = l_handed_invokable && get_precedence(l_handed_invokable) < precedence;
                    serialize_property_with_or_without_brackets( args[0]->first_adjacent().get(), needs_brackets );
                }

                // Operator
                if (!_component->token.is_null())
                {
                    _out.append(_component->token.buffer(), _component->token.m_buffer_size);
                }
                else
                {
                    _out.append(type->get_identifier());
                }

                // Right part of the expression
                {
                    // TODO: needs_brackets could be deduced if we pass args[1] to the function
                    auto r_handed_invokable = owner->get_connected_invokable(RIGHT_VALUE_PROPERTY);
                    bool needs_brackets = r_handed_invokable && get_precedence(r_handed_invokable) < precedence;
                    serialize_property_with_or_without_brackets( args[1]->first_adjacent().get(), needs_brackets );
                }
                break;
            }

            case 1:
            {
                // operator ( ... innerOperator ... )   ex:   -(a+b)

                // Operator
                const Token* token = &_component->token;

                if (!token->is_null())
                {
                    _out.append(token->prefix(), token->prefix_size());
                    _out.append(type->get_identifier());
                    _out.append(token->suffix(), token->suffix_size());
                }
                else
                {
                    _out.append(type->get_identifier());
                }

                auto inner_operator = owner->get_connected_invokable(LEFT_VALUE_PROPERTY);
                serialize_property_with_or_without_brackets( args[0]->first_adjacent().get(), inner_operator != nullptr);
                break;
            }
        }
    }
    else
    {
        serialize_func_call(_out, type, _component->get_arguments());
    }

    return _out;
}

std::string &Nodlang::serialize_func_call(std::string &_out, const fw::func_type *_signature, const std::vector<SlotRef> inputs) const
{
    _out.append(_signature->get_identifier());
    serialize_token_t(_out, Token_t::parenthesis_open);

    for (const SlotRef input_slot : inputs)
    {
        FW_EXPECT( input_slot->flags == SlotFlag_INPUT, "Must be an input" )
        if ( input_slot != inputs.front())
        {
            serialize_token_t(_out, Token_t::list_separator);
        }
        serialize_edge(_out, input_slot.get() );
    }

    serialize_token_t(_out, Token_t::parenthesis_close);
    return _out;
}

std::string &Nodlang::serialize_func_sig(std::string &_out, const fw::func_type *_signature) const
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
        serialize_type(_out, it->m_type);
    }

    serialize_token_t(_out, Token_t::parenthesis_close);
    return _out;
}

std::string &Nodlang::serialize_token_t(std::string &_out, const Token_t &_type) const
{
    return _out.append(to_string(_type));
}

std::string &Nodlang::serialize_type(std::string &_out, const fw::type *_type) const
{
    return _out.append(to_string(_type));
}

std::string& Nodlang::serialize_variable(std::string &_out, const VariableNode *_node) const
{
    InstructionNode* decl_instr = _node->get_declaration_instr().get();

    // 1. Serialize variable's type

    if (decl_instr)
    {
        // If parsed
        if (!_node->type_token.is_null())
        {
            serialize_token(_out, _node->type_token);
        }
        else // If created in the graph by the user
        {
            serialize_type(_out, _node->property()->get_type());
            _out.append(" ");
        }
    }

    // 2. Serialize variable identifier

    _out.append(_node->name);

    // 3. If variable is connected, serialize its assigned expression

    const Slot& slot = *_node->find_slot( _node->property()->id, SlotFlag_INPUT );
    if ( slot != Slot::null && decl_instr && slot.adjacent_count() != 0 )
    {
        if ( _node->assignment_operator_token.is_null() )
        { _out.append(" = "); }
        else
        { _out.append(_node->assignment_operator_token.buffer_to_string()); }

        serialize_edge(_out, &slot );
    }
    return _out;
}

std::string &Nodlang::serialize_variant(std::string &_out, const fw::variant *variant) const
{
    std::string variant_string = variant->to<std::string>();

    if (variant->get_type()->is<std::string>())
    {
        return _out.append('"' + variant_string + '"');
    }
    return _out.append(variant_string);
}

std::string &Nodlang::serialize_edge(std::string& _out, const Slot* _slot, bool recursively) const
{
    if( _slot == nullptr )
    {
        return _out;
    }

    const Slot* adjacent_slot = _slot->first_adjacent().get();

    if( adjacent_slot == nullptr )
    {
        return _out.append("not implemented yet");
    }

    const Property* property          = _slot->get_property();
    const Property* adjacent_property = adjacent_slot->get_property();

    // specific case of a Node*
    if ( adjacent_property->is_this() && adjacent_property->value()->is_initialized() )
    {
        return serialize_node(_out, (PoolID<const Node>)*adjacent_property->value() );
    }

    if (!adjacent_property->token.is_null())
    {
        _out.append( adjacent_property->token.prefix_to_string()); // FIXME: avoid std::string copy
    }

    if (recursively && adjacent_slot->node->find_slot( SlotFlag_ORDER_SECOND ) )
    {
        PoolID<InvokableComponent> compute_component = _slot->get_node()->get_component<InvokableComponent>();

        if (compute_component)
        {
            serialize_invokable(_out, compute_component.get());
        }
        else
        {
            serialize_property(_out, _slot->get_property());
        }
    }
    else if ( _slot->get_node()->get_type()->is<VariableNode>() )
    {
        _out.append( _slot->get_node()->name );
    }
    else
    {
        serialize_variant(_out, adjacent_property->value() );
    }

    if (!adjacent_property->token.is_null())
    {
        _out.append( adjacent_property->token.suffix_to_string()); // FIXME: avoid std::string copy
    }
    return _out;
}

std::string& Nodlang::serialize_node(std::string &_out, PoolID<const Node> node_id) const
{
    const Node* node = node_id.get();
    FW_ASSERT( node )

    if ( node->get_type()->is<InstructionNode>() )
    {
        return serialize_instr(_out, node_id );
    }

    if ( auto* cond_struct = fw::cast<const ConditionalStructNode>(node ) )
    {
        return serialize_cond_struct(_out, cond_struct);
    }

    if ( auto* for_loop = fw::cast<const ForLoopNode>(node ) )
    {
        return serialize_for_loop(_out, for_loop);
    }

    if ( auto* while_loop = fw::cast<const WhileLoopNode>(node ) )
    {
        return serialize_while_loop(_out, while_loop);
    }

    if ( auto* scope = node->get_component<Scope>().get() )
    {
        return serialize_scope(_out, scope);
    }

    if ( auto* literal = fw::cast<const LiteralNode>(node ) )
    {
        return serialize_property(_out, literal->value());
    }

    if ( auto* variable = fw::cast<const VariableNode>(node ) )
    {
        return serialize_variable(_out, variable);
    }

    if (auto* invokable = node->get_component<InvokableComponent>().get() )
    {
        return serialize_invokable(_out, invokable);
    }

    std::string message = "Unable to serialize node with type: ";
    message.append(node->get_type()->get_name());
    throw std::runtime_error(message);
}

std::string &Nodlang::serialize_scope(std::string &_out, const Scope *_scope) const
{
    serialize_token(_out, _scope->token_begin);
    for (PoolID<Node> child : _scope->get_owner()->children() )
    {
        serialize_node(_out, child);
    }
    return serialize_token(_out, _scope->token_end);
}

std::string& Nodlang::serialize_instr(std::string &_out, PoolID<const InstructionNode> instruction) const
{
    const Slot& root = *instruction->find_slot( ROOT_PROPERTY, SlotFlag_INPUT );
    if ( root.adjacent_count() != 0 && root.get_property()->value()->is_initialized() )
    {
        serialize_node(_out, root.first_adjacent()->node );
    }

    return serialize_token(_out, instruction->token_end);
}

std::string &Nodlang::serialize_token(std::string& _out, const Token& _token) const
{
    if (!_token.is_null() && _token.has_buffer())
    {
        _out.append(_token.buffer(), _token.m_buffer_size);
    }
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

    const Slot& init_slot = *_for_loop->find_slot( INITIALIZATION_PROPERTY, SlotFlag_INPUT );
    const Slot& cond_slot = *_for_loop->find_slot( CONDITION_PROPERTY, SlotFlag_INPUT );
    const Slot& iter_slot = *_for_loop->find_slot( ITERATION_PROPERTY, SlotFlag_INPUT );

    const Slot* init_slot_adjacent = init_slot.first_adjacent().get();
    const Slot* cond_slot_adjacent = cond_slot.first_adjacent().get();
    const Slot* iter_slot_adjacent = iter_slot.first_adjacent().get();

    if( init_slot_adjacent != nullptr )
    {
        Node* adjacent_node = init_slot_adjacent->node.get();
        if ( adjacent_node->get_type()->is<VariableNode>() )
        {
            serialize_variable(_out, fw::cast<VariableNode>(adjacent_node) );
        }
        else
        {
            serialize_edge(_out, init_slot_adjacent );
        }
    }

    if( cond_slot_adjacent != nullptr )
    {
        serialize_edge(_out, cond_slot_adjacent );
    }

    if( iter_slot_adjacent != nullptr )
    {
        serialize_edge(_out, iter_slot_adjacent);
    }

    serialize_token_t(_out, Token_t::parenthesis_close);

    // if scope
    if (const Scope* scope = _for_loop->get_condition_true_scope().get())
    {
        serialize_scope(_out, scope);
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

    if( PoolID<const InstructionNode> condition = _while_loop_node->cond_instr )
    {
        serialize_instr(_out, condition);
    }

    serialize_token_t(_out, Token_t::parenthesis_close);

    // if scope
    if ( const Scope* scope = _while_loop_node->get_condition_true_scope().get())
    {
        serialize_scope(_out, scope);
    }
    else
    {
        // When created manually, no scope is created, we serialize a fake one
        _out.append("\n{\n}\n");
    }

    return _out;
}


std::string &Nodlang::serialize_cond_struct(std::string &_out, const ConditionalStructNode *_condStruct) const
{
    // if ...
    if (_condStruct->token_if.is_null())
    {
        serialize_token_t(_out, Token_t::keyword_if);
    }
    else
    {
        serialize_token(_out, _condStruct->token_if);
    }

    // ... ( <condition> )
    serialize_token_t(_out, Token_t::parenthesis_open);
    if ( PoolID<const InstructionNode> condition = _condStruct->cond_expr )
    {
        serialize_instr(_out, condition);
    }
    serialize_token_t(_out, Token_t::parenthesis_close);

    // ... ( ... ) <scope>
    if ( const Scope* ifScope = _condStruct->get_condition_true_scope().get() )
    {
        serialize_scope(_out, ifScope);
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
    if ( !_condStruct->token_else.is_null() )
    {
        serialize_token(_out, _condStruct->token_else);
        if ( const Scope* else_scope = _condStruct->get_condition_false_scope().get() )
        {
            serialize_node(_out, else_scope->get_owner() );
        }
    }
    return _out;
}

// Language definition ------------------------------------------------------------------------------------------------------------

std::shared_ptr<const iinvokable> Nodlang::find_function(const func_type* _type) const
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
    return _out.append("serialize_property not implemented");
}

std::shared_ptr<const iinvokable> Nodlang::find_function_exact(const func_type *_signature) const
{
    auto is_exactly = [&](std::shared_ptr<const iinvokable> fct) {
        return fct->get_type()->is_exactly(_signature);
    };

    auto it = std::find_if(m_functions.begin(), m_functions.end(), is_exactly);

    if (it != m_functions.end())
    {
        return *it;
    }

    return nullptr;
}

std::shared_ptr<const iinvokable> Nodlang::find_function_fallback(const func_type *_type) const
{

    auto is_compatible = [&](std::shared_ptr<const iinvokable> _invokable) {
        return _type->is_compatible(_invokable->get_type());
    };

    auto found = std::find_if(m_functions.cbegin(), m_functions.cend(), is_compatible);

    if (found != m_functions.end())
    {
        return *found;
    }

    return nullptr;
}

std::shared_ptr<const iinvokable> Nodlang::find_operator_fct_exact(const func_type *_type) const
{
    if (!_type)
    {
        return nullptr;
    }

    auto is_exactly = [&](std::shared_ptr<const iinvokable> _invokable) {
        return _type->is_exactly(_invokable->get_type());
    };

    auto found = std::find_if(m_operators_impl.cbegin(), m_operators_impl.cend(), is_exactly);

    if (found != m_operators_impl.end())
    {
        return *found;
    }

    return nullptr;
}

std::shared_ptr<const iinvokable> Nodlang::find_operator_fct(const func_type *_type) const
{
    if (!_type)
    {
        return nullptr;
    }
    auto exact = find_operator_fct_exact(_type);
    if (!exact) return find_operator_fct_fallback(_type);
    return exact;
}

std::shared_ptr<const iinvokable> Nodlang::find_operator_fct_fallback(const func_type *_type) const
{

    auto is_compatible = [&](std::shared_ptr<const iinvokable> _invokable) {
        return _type->is_compatible(_invokable->get_type());
    };

    auto found = std::find_if(m_operators_impl.cbegin(), m_operators_impl.cend(), is_compatible);

    if (found != m_operators_impl.end())
    {
        return *found;
    }

    return nullptr;
}

void Nodlang::add_function(std::shared_ptr<const iinvokable> _invokable)
{
    m_functions.push_back(_invokable);

    const func_type *type = _invokable->get_type();

    std::string type_as_string;
    serialize_func_sig(type_as_string, type);

    // Stops if no operator having the same identifier and argument count is found
    if (!find_operator(type->get_identifier(), static_cast<Operator_t>(type->get_arg_count())))
    {
        LOG_VERBOSE("Nodlang", "add function: %s (in m_functions)\n", type_as_string.c_str());
        return;
    }

    // Register the invokable as an operator implementation
    auto found = std::find(m_operators_impl.begin(), m_operators_impl.end(), _invokable);
    FW_ASSERT(found == m_operators_impl.end())
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

std::string &Nodlang::to_string(std::string &_out, const type *_type) const
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

std::string Nodlang::to_string(const fw::type *_type) const
{
    std::string result;
    return to_string(result, _type);
}

std::string Nodlang::to_string(Token_t _token) const
{
    std::string result;
    return to_string(result, _token);
}

int Nodlang::get_precedence(const iinvokable* _invokable) const
{
    if (!_invokable)
        return std::numeric_limits<int>::min(); // default

    const func_type* type = _invokable->get_type();
    const Operator* operator_ptr = find_operator(type->get_identifier(), static_cast<Operator_t>(type->get_arg_count()));

    if (operator_ptr)
        return operator_ptr->precedence;
    return std::numeric_limits<int>::max();
}

const type * Nodlang::get_type(Token_t _token) const
{
    FW_EXPECT(is_a_type_keyword(_token), "_token_t is not a type keyword!");
    return m_type_by_token_t.find(_token)->second;
    return m_type_by_token_t.find(_token)->second;
}

Nodlang& Nodlang::get_instance()
{
    static Nodlang instance;
    return instance;
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
    delete[] source_buffer;
}

void Nodlang::ParserState::set_source_buffer(const char *str, size_t size)
{
    FW_ASSERT(source_buffer == nullptr); // should call clear() before
    FW_ASSERT(str != nullptr);

    if( size != 0 )
    {
        LOG_VERBOSE("ParserState", "Copying source buffer (%i bytes) ...\n", size);
        source_buffer = new char[size];
        memcpy(source_buffer, str, size);
    }
    source_buffer_size = size;
    ribbon.set_source_buffer(source_buffer);
}

void Nodlang::ParserState::clear()
{
    graph = nullptr;
    ribbon.clear();
    delete[] source_buffer;
    source_buffer      = nullptr;
    source_buffer_size = 0;
    while(!scope.empty())
    {
        scope.pop();
    }
}
