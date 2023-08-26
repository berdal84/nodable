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

#include "core/Pool.h"
#include "core/ConditionalStructNode.h"
#include "core/DirectedEdge.h"
#include "core/ForLoopNode.h"
#include "core/WhileLoopNode.h"
#include "core/Graph.h"
#include "core/InstructionNode.h"
#include "core/InvokableComponent.h"
#include "core/LiteralNode.h"
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
         { "int",    Token_t::keyword_int,     type::get<i16_t>()}
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


    if (!tokenize(parser_state.source.buffer, parser_state.source.size))
    {
        return false;
    }

    if (!is_syntax_valid())
    {
        return false;
    }

    high_resolution_clock::time_point tokenize_end = high_resolution_clock::now();
    LOG_MESSAGE("Parser", "%16s == %.3f ms\n", "tokenize()",  duration_cast<duration<double>>( tokenize_end - parse_begin).count() * 1000.0)

    ID<Node> program = parse_program();

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

i16_t Nodlang::to_i16(const std::string &_str)
{
    return stoi(_str);
}

Property* Nodlang::to_property(Token _token)
{
    if (_token.m_type == Token_t::identifier)
    {
        ID<VariableNode> variable = get_current_scope()->find_variable( _token.word_to_string() );

        if ( variable.get() == nullptr)
        {
            if (m_strict_mode)
            {
                LOG_ERROR("Parser", "Expecting declaration for symbol %s (strict mode) \n", _token.word_to_string().c_str())
            } else
            {
                /* when strict mode is OFF, we just create a variable with Any type */
                LOG_WARNING("Parser", "Expecting declaration for symbol %s, compilation will fail.\n",
                            _token.word_to_string().c_str())
                variable = parser_state.graph->create_variable(type::null(), _token.word_to_string(), get_current_scope() );
                variable->property()->token = std::move(_token);
                variable->set_declared(false);
            }
        }
        if ( variable.get() != nullptr )
        {
            return variable->property();
        }
        return nullptr;
    }

    ID<LiteralNode> literal;

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
            literal = parser_state.graph->create_literal<i16_t>();
            literal->value()->set(to_i16(_token.word_to_string())); // FIXME: avoid std::string copy
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
        literal->value()->token = std::move(_token);
        return literal->value();
    }

    LOG_VERBOSE("Parser", "Unable to perform token_to_property for token %s!\n", _token.word_to_string().c_str())
    return {};
}

Property* Nodlang::parse_binary_operator_expression(unsigned short _precedence, Property* _left)
{

    FW_ASSERT( _left != nullptr );

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
    Property* right = parse_expression(ope->precedence, nullptr);

    if (right == nullptr)
    {
        LOG_VERBOSE("Parser", "parseBinaryOperationExpression... " KO " (right expression is nullptr)\n")
        rollback_transaction();
        return nullptr;
    }

    // Create a function signature according to ltype, rtype and operator word
    func_type *type = new func_type(ope->identifier);
    type->set_return_type(type::any());
    type->push_args(_left->get_type(), right->get_type());

    ID<InvokableComponent> component;
    ID<Node> binary_op;

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
    parser_state.graph->connect(_left, component->get_l_handed_val());
    parser_state.graph->connect(right, component->get_r_handed_val());

    commit_transaction();
    LOG_VERBOSE("Parser", "parse binary operation expr... " OK "\n")
    return binary_op->get_prop(k_value_property_name);
}

Property* Nodlang::parse_unary_operator_expression(unsigned short _precedence)
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
    Property* value = parse_atomic_expression();

    if ( value == nullptr )
    {
        value = parse_parenthesis_expression();
    }

    if ( value == nullptr )
    {
        LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (right expression is nullptr)\n")
        rollback_transaction();
        return nullptr;
    }

    // Create a function signature
    auto* type = new func_type(operator_token.word_to_string());  // FIXME: avoid std::string copy
    type->set_return_type(type::any());
    type->push_args(value->get_type());

    ID<InvokableComponent> component;
    ID<Node> node;

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

    parser_state.graph->connect(value, component->get_l_handed_val());
    Property* result = node->get_prop(k_value_property_name);

    LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " OK "\n")
    commit_transaction();

    return result;
}

Property* Nodlang::parse_atomic_expression()
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

    Property* result = to_property(token);

    if ( result != nullptr )
    {
        commit_transaction();
        LOG_VERBOSE("Parser", "parse atomic expr... " OK "\n")
        return result;
    }

    rollback_transaction();
    LOG_VERBOSE("Parser", "parse atomic expr... " KO " (result is nullptr)\n")

    return nullptr;
}

Property* Nodlang::parse_parenthesis_expression()
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

    Property* result = parse_expression();
    if ( result != nullptr )
    {
        Token token = parser_state.ribbon.eat();
        if (token.m_type != Token_t::parenthesis_close)
        {
            LOG_VERBOSE("Parser", "%s \n", parser_state.ribbon.to_string().c_str())
            LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " ( \")\" expected instead of %s )\n",
                        token.word_to_string().c_str())
            rollback_transaction();
        } else
        {
            LOG_VERBOSE("Parser", "parse parenthesis expr..." OK "\n")
            commit_transaction();
        }
    } else
    {
        LOG_VERBOSE("Parser", "parse parenthesis expr..." KO ", expression in parenthesis is nullptr.\n")
        rollback_transaction();
    }
    return result;
}

ID<InstructionNode> Nodlang::parse_instr()
{
    start_transaction();

    Property* expression = parse_expression();

    if ( expression == nullptr )
    {
        LOG_VERBOSE("Parser", "parse instruction " KO " (parsed is nullptr)\n")
        rollback_transaction();
        return {};
    }

    ID<InstructionNode> instr_node = parser_state.graph->create_instr();

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
            return {};
        }
    }

    parser_state.graph->connect(expression->owner().get(), instr_node.get());

    LOG_VERBOSE("Parser", "parse instruction " OK "\n")
    commit_transaction();
    return instr_node->id();
}

ID<Node> Nodlang::parse_program()
{
    start_transaction();

    parser_state.graph->clear();
    ID<Node> root = parser_state.graph->create_root();
    ID<Scope> program_scope = root->get_component<Scope>();
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

ID<Node> Nodlang::parse_scope()
{
    ID<Node> result;

    start_transaction();

    if (parser_state.ribbon.eat_if(Token_t::scope_begin).is_null())
    {
        rollback_transaction();
    }
    else
    {
        ID<Node>  scope_node = parser_state.graph->create_scope();
        ID<Scope> scope      = scope_node->get_component<Scope>();
        /*
         * link scope with parent_scope.
         * They must be linked in order to find_variables recursively.
         */
        Scope* parent_scope = get_current_scope().get();
        if (parent_scope)
        {
            ID<Node> parent_scope_node = parent_scope->get_owner();
            parser_state.graph->connect({scope_node.get(), Edge_t::IS_CHILD_OF, parent_scope_node.get() });
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
            result = scope_node->id();
        }

        parser_state.scope.pop();
    }
    return result;
}

ID<Scope> Nodlang::parse_code_block(ID<Scope> curr_scope)
{
    FW_ASSERT(curr_scope);

    start_transaction();

    while (parser_state.ribbon.can_eat())
    {
        if ( InstructionNode* instr_node = parse_instr().get() )
        {
            parser_state.graph->connect({instr_node, Edge_t::IS_CHILD_OF, get_current_scope_node().get() });
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

    if (curr_scope->get_owner()->children.empty())
    {
        rollback_transaction();
        return {};
    }
    else
    {
        commit_transaction();
        return curr_scope;
    }
}

Property* Nodlang::parse_expression(unsigned short _precedence, Property* _leftOverride)
{
    LOG_VERBOSE("Parser", "parse expr...\n")
    LOG_VERBOSE("Parser", "%s \n", parser_state.ribbon.to_string().c_str())

    if (!parser_state.ribbon.can_eat())
    {
        LOG_VERBOSE("Parser", "parse expr..." KO " (unable to eat a single token)\n")
        return _leftOverride;
    }

    /*
		Get the left handed operand
	*/
    Property* left = _leftOverride;
    if ( left == nullptr) left = parse_parenthesis_expression();
    if ( left == nullptr) left = parse_unary_operator_expression(_precedence);
    if ( left == nullptr) left = parse_function_call();
    if ( left == nullptr) left = parse_variable_declaration();
    if ( left == nullptr) left = parse_atomic_expression();

    if (!parser_state.ribbon.can_eat())
    {
        LOG_VERBOSE("Parser", "parse expr... " OK " (last token reached)\n")
    }

    Property* result;

    /*
		Get the right handed operand
	*/
    if ( left != nullptr )
    {
        LOG_VERBOSE("Parser", "parse expr... left parsed, we parse right\n")
        if ( Property* binResult = parse_binary_operator_expression(_precedence, left) )
        {
            LOG_VERBOSE("Parser", "parse expr... right parsed, recursive call\n")
            result = parse_expression(_precedence, binResult);
        } else
        {
            result = left;
        }

    } else
    {
        LOG_VERBOSE("Parser", "parse expr... left is nullptr, we return it\n")
        result = left;
    }

    return result;
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

Property* Nodlang::parse_function_call()
{
    LOG_VERBOSE("Parser", "parse function call...\n")

    // Check if the minimum token count required is available ( 0: identifier, 1: open parenthesis, 2: close parenthesis)
    if (!parser_state.ribbon.can_eat(3))
    {
        LOG_VERBOSE("Parser", "parse function call... " KO " aborted, not enough tokens.\n")
        return {};
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
            return {};
        }
    }
    std::vector<Property* > args;

    // Declare a new function prototype
    func_type signature(fct_id);
    signature.set_return_type(type::any());

    bool parsingError = false;
    while (!parsingError && parser_state.ribbon.can_eat() &&
            parser_state.ribbon.peek().m_type != Token_t::parenthesis_close)
    {

        if ( Property* property = parse_expression() )
        {
            args.push_back(property);            // store argument as property (already parsed)
            signature.push_arg(property->get_type());// add a new argument type to the proto.
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
        return {};
    }


    // Find the prototype in the language library
    std::shared_ptr<const iinvokable> invokable = find_function(&signature);

    auto connect_arg = [&](const func_type *_sig, Node *_node, size_t _arg_index) -> void {// lambda to connect input property to node for a specific argument index.
        Property* src_property = args.at(_arg_index);
        Property* dst_property = _node->props.get_input_at(_arg_index);
        FW_ASSERT(dst_property)
        parser_state.graph->connect(src_property, dst_property);
    };

    ID<Node> fct_node_id;
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
    for (size_t argIndex = 0; argIndex < signature.get_arg_count(); argIndex++)
    {
        connect_arg(&signature, fct_node, argIndex);
    }

    commit_transaction();
    LOG_VERBOSE("Parser", "parse function call... " OK "\n")

    return fct_node->get_prop(k_value_property_name);
}

ID<Scope> Nodlang::get_current_scope()
{
    FW_ASSERT( parser_state.scope.top() );
    return parser_state.scope.top();
}

ID<Node> Nodlang::get_current_scope_node()
{
    FW_ASSERT( parser_state.scope.top() );
    return parser_state.scope.top()->get_owner();
}

ID<ConditionalStructNode> Nodlang::parse_conditional_structure()
{
    LOG_VERBOSE("Parser", "try to parse conditional structure...\n")
    start_transaction();

    bool success = false;
    ID<InstructionNode>       condition;
    ID<Node>                  scopeIf;
    ID<ConditionalStructNode> condStruct;
    ID<ConditionalStructNode> else_cond_struct;

    Token if_token = parser_state.ribbon.eat_if(Token_t::keyword_if);
    if ( if_token.is_null() )
    {
        return {};
    }

    condStruct = parser_state.graph->create_cond_struct();
    Node* scope_node = get_current_scope()->get_owner().get();

    parser_state.graph->connect({condStruct.get(), Edge_t::IS_CHILD_OF, scope_node});
    parser_state.scope.push(condStruct->get_component<Scope>()->id());

    condStruct->token_if  = parser_state.ribbon.get_eaten();

    if (!parser_state.ribbon.eat_if(Token_t::parenthesis_open).is_null())
    {
        bool empty_parenthesis = !parser_state.ribbon.eat_if(Token_t::parenthesis_close).is_null();
        if (!empty_parenthesis && (condition = parse_instr()))
        {
            condition->set_name("Condition");
            condition->set_name("Cond.");
            condStruct->cond_expr = condition;
            parser_state.graph->connect(condition->as_prop(), condStruct->condition_property());
        }

        if ( empty_parenthesis || ( condition.get() && !parser_state.ribbon.eat_if(Token_t::parenthesis_close).is_null()))
        {
            scopeIf = parse_scope();
            if ( scopeIf.get() != nullptr )
            {
                if (!parser_state.ribbon.eat_if(Token_t::keyword_else).is_null())
                {
                    condStruct->token_else = parser_state.ribbon.get_eaten();

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
        return condStruct;
    }

    parser_state.graph->destroy( else_cond_struct );
    parser_state.graph->destroy( scopeIf );
    parser_state.graph->destroy( condition );
    parser_state.graph->destroy( condStruct );
    rollback_transaction();
    return {};
}

ID<ForLoopNode> Nodlang::parse_for_loop()
{
    bool success = false;
    ID<ForLoopNode> for_loop_node;
    start_transaction();

    Token token_for = parser_state.ribbon.eat_if(Token_t::keyword_for);

    if (!token_for.is_null())
    {
        for_loop_node = parser_state.graph->create_for_loop();
        parser_state.graph->connect({for_loop_node.get(), Edge_t::IS_CHILD_OF, get_current_scope()->get_owner().get() });
        parser_state.scope.push(for_loop_node->get_component<Scope>()->id());

        for_loop_node->token_for = token_for;

        LOG_VERBOSE("Parser", "parse FOR (...) block...\n")
        Token open_bracket = parser_state.ribbon.eat_if(Token_t::parenthesis_open);
        if (open_bracket.is_null())
        {
            LOG_ERROR("Parser", "Unable to find open bracket after for keyword.\n")
        } else
        {
            ID<InstructionNode> init_instr = parse_instr();
            if (!init_instr.get())
            {
                LOG_ERROR("Parser", "Unable to find initial instruction.\n")
            } else
            {
                init_instr->set_name("Initialisation");
                parser_state.graph->connect(init_instr->as_prop(), for_loop_node->get_init_expr());
                for_loop_node->init_instr = init_instr;

                ID<InstructionNode> cond_instr = parse_instr();
                if (!cond_instr.get())
                {
                    LOG_ERROR("Parser", "Unable to find condition instruction.\n")
                } else
                {
                    cond_instr->set_name("Condition");
                    parser_state.graph->connect(cond_instr->as_prop(), for_loop_node->condition_property());
                    for_loop_node->cond_instr = cond_instr;

                    ID<InstructionNode> iter_instr = parse_instr();
                    if (!iter_instr.get())
                    {
                        LOG_ERROR("Parser", "Unable to find iterative instruction.\n")
                    } else
                    {
                        iter_instr->set_name("Iteration");
                        parser_state.graph->connect(iter_instr->as_prop(), for_loop_node->get_iter_expr() );
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

ID<WhileLoopNode> Nodlang::parse_while_loop()
{
    bool success = false;
    ID<WhileLoopNode> while_loop_node;
    start_transaction();

    Token token_while = parser_state.ribbon.eat_if(Token_t::keyword_while);

    if (!token_while.is_null())
    {
        while_loop_node = parser_state.graph->create_while_loop();
        parser_state.graph->connect({while_loop_node.get(), Edge_t::IS_CHILD_OF, get_current_scope()->get_owner().get() });
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
            while_loop_node->cond_instr = cond_instr->id();
            parser_state.graph->connect(cond_instr->as_prop(), while_loop_node->condition_property());

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
    return {};
}

Property* Nodlang::parse_variable_declaration()
{

    if (!parser_state.ribbon.can_eat(2))
        return {};

    start_transaction();

    Token type_token = parser_state.ribbon.eat();
    Token identifier_token = parser_state.ribbon.eat();

    if (type_token.is_keyword_type() && identifier_token.m_type == Token_t::identifier)
    {
        const type* type = get_type(type_token.m_type);
        ID<VariableNode> variable_id = parser_state.graph->create_variable(type, identifier_token.word_to_string(), get_current_scope() );
        VariableNode* variable{ variable_id.get() };
        variable->set_declared(true);
        variable->type_token = type_token;
        variable->identifier_token.transfer_prefix_and_suffix_from(&identifier_token);
        variable->property()->token = identifier_token;

        // try to parse assignment
        Token operator_token = parser_state.ribbon.eat_if(Token_t::operator_);
        if (!operator_token.is_null() && operator_token.word_size() == 1 && *operator_token.word() == '=')
        {
            Property* expression_result = parse_expression();
            if (expression_result &&
                type::is_implicitly_convertible(expression_result->get_type(), variable->type() ) )
            {
                parser_state.graph->connect(expression_result, variable);
                variable->assignment_operator_token = operator_token;
            }
            else
            {
                LOG_ERROR("Parser", "Unable to parse expression to assign %s\n", identifier_token.word_to_string().c_str())
                rollback_transaction();
                parser_state.graph->destroy( variable->id() );
                return {};
            }
        }

        commit_transaction();
        return variable->property();
    }

    rollback_transaction();
    return {};
}

//---------------------------------------------------------------------------------------------------------------------------
// [SECTION] C. Serializer --------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------

std::string &Nodlang::serialize_invokable(std::string &_out, const InvokableComponent *_component) const
{
    const func_type *type = _component->get_func_type();

    if (_component->is_operator())
    {
        // generic serialize property lambda
        auto serialize_property_with_or_without_brackets = [this, &_out](const Property* property, bool needs_brackets)
        {
            if (!needs_brackets)
            {
                serialize_property(_out, property);
                return;
            }
            serialize_token_t(_out, Token_t::parenthesis_open);
            serialize_property(_out, property);
            serialize_token_t(_out, Token_t::parenthesis_close);
        };

        const Node* owner = _component->get_owner().get();
        const std::vector<Property*>& args = _component->get_args();
        int precedence = get_precedence(_component->get_function());

        switch (type->get_arg_count())
        {
            case 2:
            {
                // Left part of the expression
                {
                    auto l_handed_invokable = owner->get_connected_invokable(args[0]);
                    bool needs_brackets = l_handed_invokable && get_precedence(l_handed_invokable) < precedence;

                    serialize_property_with_or_without_brackets(args[0], needs_brackets);
                }

                // Operator
                if (!_component->token.is_null())
                {
                    _out.append(_component->token.buffer(), _component->token.m_buffer_size);
                } else
                {
                    _out.append(type->get_identifier());
                }

                // Right part of the expression
                {
                    auto r_handed_invokable = owner->get_connected_invokable(args[1]);
                    bool needs_brackets = r_handed_invokable && get_precedence(r_handed_invokable) < precedence;
                    serialize_property_with_or_without_brackets(args[1], needs_brackets);
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

                auto inner_operator = owner->get_connected_invokable(args[0]);
                serialize_property_with_or_without_brackets(args[0], inner_operator != nullptr);
                break;
            }
        }
    } else
    {
        serialize_func_call(_out, type, _component->get_args());
    }

    return _out;
}

std::string &Nodlang::serialize_func_call(std::string &_out, const fw::func_type *_signature, const std::vector<Property* > &_args) const
{
    _out.append(_signature->get_identifier());
    serialize_token_t(_out, Token_t::parenthesis_open);

    for (auto it = _args.begin(); it != _args.end(); it++)
    {
        serialize_property(_out, *it);

        if (*it != _args.back())
        {
            serialize_token_t(_out, Token_t::list_separator);
        }
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

    const Property* value = _node->property();
    if (decl_instr && value->has_input_connected())
    {
        _out.append(_node->assignment_operator_token.is_null() ? " = " : _node->assignment_operator_token.buffer_to_string());
        serialize_property(_out, value);
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

std::string &Nodlang::serialize_property(std::string &_out, const Property *_property, bool recursively) const
{
    // specific case of a Node*
    if (_property->get_type()->is<ID<Node>>())
    {
        if ( (*_property)->is_initialized() )
        {
            return serialize_node(_out, (i32_t)*_property->value() );
        }
    }


    if (!_property->token.is_null())
    {
        _out.append(_property->token.prefix_to_string()); // FIXME: avoid std::string copy
    }

    Node* owner = _property->owner().get();
    if (recursively && owner && _property->allows_connection(Way_In) && owner->is_connected_with(_property))
    {
        Property*           src_property      = _property->get_input();
        ID<InvokableComponent> compute_component = src_property->owner()->get_component<InvokableComponent>();

        if (compute_component)
        {
            serialize_invokable(_out, compute_component.get());
        }
        else
        {
            serialize_property(_out, src_property, false);
        }
    }
    else if ( owner->get_type()->is<VariableNode>() )
    {
        _out.append( owner->name );
    }
    else
    {
        serialize_variant(_out, _property->value() );
    }

    if (!_property->token.is_null())
    {
        _out.append(_property->token.suffix_to_string()); // FIXME: avoid std::string copy
    }
    return _out;
}

std::string& Nodlang::serialize_node(std::string &_out, ID<const Node> node_id) const
{
    const Node* node{ node_id.get() };
    FW_ASSERT( node )
    const type* type = node->get_type();
    if (auto* instr = fw::cast<const InstructionNode>(node ) )
    {
        return serialize_instr(_out, instr);
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

    std::string message = "Unable to serialize ";
    message.append(type->get_name());
    throw std::runtime_error(message);
}

std::string &Nodlang::serialize_scope(std::string &_out, const Scope *_scope) const
{
    serialize_token(_out, _scope->token_begin);
    for (ID<const Node> each_child : _scope->get_owner()->children)
    {
        serialize_node(_out, each_child );
    }
    return serialize_token(_out, _scope->token_end);
}

std::string& Nodlang::serialize_instr(std::string &_out, const InstructionNode *_instruction) const
{
    FW_EXPECT(_instruction != nullptr, "IntructionNode should NOT be nullptr");

    Property* instr_root = _instruction->root();
    if (instr_root->has_input_connected() && (*instr_root)->is_initialized())
    {
        serialize_node(_out, instr_root->owner() );
    }

    return serialize_token(_out, _instruction->token_end);
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

    // TODO: I don't like this if/else, should be implicit. Serialize Property* must do it.
    //       More work to do to know if expression is a declaration or not.

    Node* input_node = _for_loop->get_init_expr()->get_input()->owner().get();
    if ( auto * input_variable = fw::cast<const VariableNode>( input_node ) )
    {
        serialize_variable(_out, input_variable );
    }
    else if ( const InstructionNode* init_instr = _for_loop->init_instr.get())
    {
        serialize_instr(_out, init_instr);
    }

    if( const InstructionNode* condition = _for_loop->cond_instr.get() )
    {
        serialize_instr(_out, condition);
    }

    if( const Property* iter_instr = _for_loop->get_iter_expr() )
    {
        serialize_property(_out, iter_instr);
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

    if( const InstructionNode* condition = _while_loop_node->cond_instr.get() )
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
    if ( const InstructionNode* condition = _condStruct->cond_expr.get() )
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

Nodlang::ParserState::ParserState()
    : graph(nullptr)
    , source({nullptr, 0})
{}

Nodlang::ParserState::~ParserState()
{
    delete[] source.buffer;
}

void Nodlang::ParserState::set_source_buffer(const char *str, size_t size)
{
    FW_ASSERT(source.buffer == nullptr); // should call clear() before
    FW_ASSERT(str != nullptr);

    if( size != 0 )
    {
        LOG_VERBOSE("ParserState", "Copying source buffer (%i bytes) ...\n", size);
        source.buffer = new char[size];
        memcpy(source.buffer, str, size);
    }
    source.size = size;
    ribbon.set_source_buffer(source.buffer);
}

void Nodlang::ParserState::clear()
{
    graph = nullptr;
    ribbon.clear();
    delete[] source.buffer;
    source.buffer = nullptr;
    source.size = 0;
    while(!scope.empty())
    {
        scope.pop();
    }
}
