//---------------------------------------------------------------------------------------------------------------------------
// Nodlang.cpp
// This file is structured in 3 parts, use Ctrl + F to search:
//  [SECTION] A. Declaration (types, keywords, etc.)
//  [SECTION] B. Parser
//  [SECTION] C. Serializer
//---------------------------------------------------------------------------------------------------------------------------

#include <ndbl/core/language/Nodlang.h>

#include <algorithm>
#include <memory>
#include <regex>
#include <sstream>
#include <string>

#include "fw/core/String.h"
#include "fw/core/System.h"
#include <fw/core/reflection/reflection>
#include "fw/core/Log.h"

#include <ndbl/core/ConditionalStructNode.h>
#include <ndbl/core/ConditionalStructNode.h>
#include <ndbl/core/DirectedEdge.h>
#include <ndbl/core/ForLoopNode.h>
#include <ndbl/core/GraphNode.h>
#include <ndbl/core/GraphNode.h>
#include <ndbl/core/InstructionNode.h>
#include <ndbl/core/InstructionNode.h>
#include <ndbl/core/InvokableComponent.h>
#include <ndbl/core/InvokableComponent.h>
#include <ndbl/core/LiteralNode.h>
#include <ndbl/core/LiteralNode.h>
#include <ndbl/core/Property.h>
#include <ndbl/core/Property.h>
#include <ndbl/core/Scope.h>
#include <ndbl/core/Scope.h>
#include <ndbl/core/VariableNode.h>
#include <ndbl/core/VariableNode.h>
#include <ndbl/core/language/Nodlang_biology.h>
#include <ndbl/core/language/Nodlang_math.h>

using namespace ndbl;
using namespace fw;

//---------------------------------------------------------------------------------------------------------------------------
// [SECTION] A. Declaration -------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------

Nodlang::Nodlang(bool _strict)
    : m_strict_mode(_strict)
    , m_graph(nullptr)
{

    // A.1. Define regular expressions
    //--------------------------------

    // ignored
    add_regex(std::regex("^(//(.+?)($|\n))"), Token_t::ignore);// Single line
    add_regex(std::regex("^(/\\*(.+?)\\*/)"), Token_t::ignore);// Multi line
    add_char('\t', Token_t::ignore);
    add_char(' ' , Token_t::ignore);

    // keywords
    add_string("if",       Token_t::keyword_if);
    add_string("else",     Token_t::keyword_else);
    add_string("for",      Token_t::keyword_for);
    add_string("operator", Token_t::keyword_operator);
    add_type(type::get<bool>(),        Token_t::keyword_bool,   "bool");
    add_type(type::get<std::string>(), Token_t::keyword_string, "string");
    add_type(type::get<double>(),      Token_t::keyword_double, "double");
    add_type(type::get<i16_t>(),       Token_t::keyword_int,    "int");

    // punctuation
    add_char('{', Token_t::scope_begin);
    add_char('}', Token_t::scope_end);
    add_char('(', Token_t::fct_params_begin);
    add_char(')', Token_t::fct_params_end);
    add_char(',', Token_t::fct_params_separator);
    add_char(';', Token_t::end_of_instruction);

#if (WIN32)
    add_char('\n', Token_t::end_of_line); // <--- should be '\n\r' not handled yet.
#elif(__APPLE__ || __linux__)
    add_char('\n', Token_t::end_of_line); // <--- We do not handle old mac '\r'.
#endif

    // literals
    // USE OTHER METHOD: insert(std::regex("^(true|false)")                , Token_t::literal_bool  , type::get<bool>());
    add_regex(std::regex(R"(("[^"]*"))"), Token_t::literal_string, type::get<std::string>());
    add_regex(std::regex("(0|([1-9][0-9]*))(\\.[0-9]+)"), Token_t::literal_double, type::get<double>());
    add_regex(std::regex("(0|([1-9][0-9]*))"), Token_t::literal_int, type::get<i16_t>());

    // identifier
    add_regex(std::regex("([a-zA-Z_]+[a-zA-Z0-9]*)"), Token_t::identifier);

    // operators
    add_regex(std::regex("(<=>)"), Token_t::operator_);                           // 3 chars
    add_regex(std::regex("([=\\|&]{2}|(<=)|(>=)|(=>)|(!=))"), Token_t::operator_);// 2 chars
    add_regex(std::regex("[/+\\-*!=<>]"), Token_t::operator_);                    // single char

    add_operator("-", Operator_t::Unary, 5);// --------- unary (sorted by precedence)
    add_operator("!", Operator_t::Unary, 5);

    add_operator("/", Operator_t::Binary, 20);// ------- binary (sorted by precedence)
    add_operator("*", Operator_t::Binary, 20);
    add_operator("+", Operator_t::Binary, 10);
    add_operator("-", Operator_t::Binary, 10);
    add_operator("||", Operator_t::Binary, 10);
    add_operator("&&", Operator_t::Binary, 10);
    add_operator(">=", Operator_t::Binary, 10);
    add_operator("<=", Operator_t::Binary, 10);
    add_operator("=>", Operator_t::Binary, 10);
    add_operator("==", Operator_t::Binary, 10);
    add_operator("<=>", Operator_t::Binary, 10);
    add_operator("!=", Operator_t::Binary, 10);
    add_operator(">", Operator_t::Binary, 10);
    add_operator("<", Operator_t::Binary, 10);
    add_operator("=", Operator_t::Binary, 0);

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
    m_token_ribbon.rollbackTransaction();
}

void Nodlang::start_transaction()
{
    m_token_ribbon.startTransaction();
}

void Nodlang::commit_transaction()
{
    m_token_ribbon.commitTransaction();
}

bool Nodlang::parse(const std::string &_source_code, GraphNode *_graphNode)
{
    m_graph = _graphNode;
    m_token_ribbon.clear();

    LOG_VERBOSE("Parser", "Trying to evaluate evaluated: <expr>%s</expr>\"\n", _source_code.c_str())
    LOG_MESSAGE("Parser", "Tokenization ...\n")


    if (!tokenize(_source_code))
    {
        return false;
    }

    if (!is_syntax_valid())
    {
        return false;
    }

    Node *program = parse_program();

    if (program == nullptr)
    {
        LOG_WARNING("Parser", "Unable to generate program tree.\n")
        return false;
    }

    if (m_token_ribbon.canEat())
    {
        m_graph->clear();
        LOG_WARNING("Parser", "Unable to generate a full program tree.\n")
        LOG_MESSAGE("Parser", "--- Token Ribbon begin ---\n");
        for (const auto &each_token: m_token_ribbon.tokens)
        {
            LOG_MESSAGE("Parser", "token idx %i: %s\n", each_token->m_index, Token::to_JSON(each_token).c_str());
        }
        LOG_MESSAGE("Parser", "--- Token Ribbon end ---\n");
        auto curr_token = m_token_ribbon.peekToken();
        LOG_ERROR("Parser", "Couldn't go further than token %llu: %s\n", curr_token->m_index, Token::to_JSON(curr_token).c_str())
        return false;
    }

    // We unset dirty, since we did a lot of connections but we don't want any update now
    auto &nodes = m_graph->get_node_registry();
    for (auto eachNode: nodes)
        eachNode->set_dirty(false);

    LOG_MESSAGE("Parser", "Program tree updated.\n", _source_code.c_str())
    LOG_VERBOSE("Parser", "Source code: <expr>%s</expr>\"\n", _source_code.c_str())
    return true;
}

bool Nodlang::to_bool(const std::string &_str)
{
    return _str == std::string("true");
}

std::string Nodlang::to_unquoted_string(const std::string &_quoted_str)
{
    NDBL_ASSERT(_quoted_str.size() >= 2);
    NDBL_ASSERT(_quoted_str.front() == '\"');
    NDBL_ASSERT(_quoted_str.back() == '\"');
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

Property *Nodlang::to_property(std::shared_ptr<Token> _token)
{
    if (_token->m_type == Token_t::identifier)
    {
        VariableNode *variable = get_current_scope()->find_variable(_token->get_word());

        if (variable == nullptr)
        {
            if (m_strict_mode)
            {
                LOG_ERROR("Parser", "Expecting declaration for symbol %s (strict mode) \n", _token->get_word().c_str())
            } else
            {
                /* when strict mode is OFF, we just create a variable with Any type */
                LOG_WARNING("Parser", "Expecting declaration for symbol %s, compilation will fail.\n", _token->get_word().c_str())
                variable = m_graph->create_variable(type::null, _token->get_word(), get_current_scope());
                variable->get_value()->set_src_token(_token);
                variable->set_declared(false);
            }
        }
        if (variable)
        {
            return variable->get_value();
        }
        return nullptr;
    }

    LiteralNode *literal = nullptr;

    switch (_token->m_type)
    {
        case Token_t::literal_bool:
        {
            literal = m_graph->create_literal(type::get<bool>());
            literal->set_value(to_bool(_token->get_word()));
            break;
        }

        case Token_t::literal_int:
        {
            literal = m_graph->create_literal(type::get<i16_t>());
            literal->set_value(to_i16(_token->get_word()));
            break;
        }

        case Token_t::literal_double:
        {
            literal = m_graph->create_literal(type::get<double>());
            literal->set_value(to_double(_token->get_word()));
            break;
        }

        case Token_t::literal_string:
        {
            literal = m_graph->create_literal(type::get<std::string>());
            literal->set_value(to_unquoted_string(_token->get_word()));
            break;
        }

        default:;
    }

    if (literal)
    {
        Property *result = literal->get_value();
        result->set_src_token(_token);
        return result;
    }

    LOG_VERBOSE("Parser", "Unable to perform token_to_property for token %s!\n", _token->get_word().c_str())
    return nullptr;
}

Property *Nodlang::parse_binary_operator_expression(unsigned short _precedence, Property *_left)
{

    assert(_left != nullptr);

    LOG_VERBOSE("Parser", "parse binary operation expr...\n")
    LOG_VERBOSE("Parser", "%s \n", m_token_ribbon.toString().c_str())

    Property *result = nullptr;

    if (!m_token_ribbon.canEat(2))
    {
        LOG_VERBOSE("Parser", "parse binary operation expr...... " KO " (not enought tokens)\n")
        return nullptr;
    }

    start_transaction();
    std::shared_ptr<Token> operatorToken = m_token_ribbon.eatToken();
    std::shared_ptr<Token> operandToken = m_token_ribbon.peekToken();

    // Structure check
    const bool isValid = _left != nullptr &&
                         operatorToken->m_type == Token_t::operator_ &&
                         operandToken->m_type != Token_t::operator_;

    if (!isValid)
    {
        rollback_transaction();
        LOG_VERBOSE("Parser", "parse binary operation expr... " KO " (Structure)\n")
        return nullptr;
    }

    auto word = operatorToken->get_word();
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
    auto right = parse_expression(ope->precedence, nullptr);

    if (!right)
    {
        LOG_VERBOSE("Parser", "parseBinaryOperationExpression... " KO " (right expression is nullptr)\n")
        rollback_transaction();
        return nullptr;
    }

    // Create a function signature according to ltype, rtype and operator word
    func_type *type = new func_type(ope->identifier);
    type->set_return_type(type::any);
    type->push_args(_left->get_type(), right->get_type());

    InvokableComponent *component;
    Node *binary_op;

    if (auto invokable = find_operator_fct(type))
    {
        // concrete operator
        binary_op = m_graph->create_operator(invokable.get());
        component = binary_op->get<InvokableComponent>();
        delete type;
    } else if (type)
    {
        // abstract operator
        binary_op = m_graph->create_abstract_operator(type);
        component = binary_op->get<InvokableComponent>();
    } else
    {
        LOG_VERBOSE("Parser", "parse binary operation expr... " KO " no signature\n")
        rollback_transaction();
        return nullptr;
    }

    component->set_source_token(operatorToken);
    m_graph->connect(_left, component->get_l_handed_val());
    m_graph->connect(right, component->get_r_handed_val());

    commit_transaction();
    LOG_VERBOSE("Parser", "parse binary operation expr... " OK "\n")
    return binary_op->props()->get(k_value_property_name);
}

Property *Nodlang::parse_unary_operator_expression(unsigned short _precedence)
{
    LOG_VERBOSE("Parser", "parseUnaryOperationExpression...\n")
    LOG_VERBOSE("Parser", "%s \n", m_token_ribbon.toString().c_str())

    if (!m_token_ribbon.canEat(2))
    {
        LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (not enough tokens)\n")
        return nullptr;
    }

    start_transaction();
    std::shared_ptr<Token> operator_token = m_token_ribbon.eatToken();

    // Check if we get an operator first
    if (operator_token->m_type != Token_t::operator_)
    {
        rollback_transaction();
        LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (operator not found)\n")
        return nullptr;
    }

    // Parse expression after the operator
    Property *value = parse_atomic_expression();

    if (value == nullptr)
    {
        value = parse_parenthesis_expression();
    }

    if (value == nullptr)
    {
        LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (right expression is nullptr)\n")
        rollback_transaction();
        return nullptr;
    }

    // Create a function signature
    func_type *type = new func_type(operator_token->get_word());
    type->set_return_type(type::any);
    type->push_args(value->get_type());

    InvokableComponent *component;
    Node *node;

    if (auto invokable = find_operator_fct(type))
    {
        node = m_graph->create_operator(invokable.get());
        delete type;
    } else if (type)
    {
        node = m_graph->create_abstract_operator(type);
    } else
    {
        LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (no signature found)\n")
        rollback_transaction();
        return nullptr;
    }

    component = node->get<InvokableComponent>();
    component->set_source_token(operator_token);

    m_graph->connect(value, component->get_l_handed_val());
    Property *result = node->props()->get(k_value_property_name);

    LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " OK "\n")
    commit_transaction();

    return result;
}

Property *Nodlang::parse_atomic_expression()
{
    LOG_VERBOSE("Parser", "parse atomic expr... \n")

    if (!m_token_ribbon.canEat())
    {
        LOG_VERBOSE("Parser", "parse atomic expr... " KO "(not enough tokens)\n")
        return nullptr;
    }

    start_transaction();
    std::shared_ptr<Token> token = m_token_ribbon.eatToken();

    if (token->m_type == Token_t::operator_)
    {
        LOG_VERBOSE("Parser", "parse atomic expr... " KO "(token is an operator)\n")
        rollback_transaction();
        return nullptr;
    }

    auto result = to_property(token);

    if (result != nullptr)
    {
        commit_transaction();
        LOG_VERBOSE("Parser", "parse atomic expr... " OK "\n")
    } else
    {
        rollback_transaction();
        LOG_VERBOSE("Parser", "parse atomic expr... " KO " (result is nullptr)\n")
    }

    return result;
}

Property *Nodlang::parse_parenthesis_expression()
{
    LOG_VERBOSE("Parser", "parse parenthesis expr...\n")
    LOG_VERBOSE("Parser", "%s \n", m_token_ribbon.toString().c_str())

    if (!m_token_ribbon.canEat())
    {
        LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " no enough tokens.\n")
        return nullptr;
    }

    start_transaction();
    std::shared_ptr<Token> currentToken = m_token_ribbon.eatToken();
    if (currentToken->m_type != Token_t::fct_params_begin)
    {
        LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " open bracket not found.\n")
        rollback_transaction();
        return nullptr;
    }

    Property *result = parse_expression();
    if (result)
    {
        std::shared_ptr<Token> token = m_token_ribbon.eatToken();
        if (token->m_type != Token_t::fct_params_end)
        {
            LOG_VERBOSE("Parser", "%s \n", m_token_ribbon.toString().c_str())
            LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " ( \")\" expected instead of %s )\n", token->get_word().c_str())
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

InstructionNode *Nodlang::parse_instr()
{
    start_transaction();

    Property *expression = parse_expression();

    if (!expression)
    {
        LOG_VERBOSE("Parser", "parse instruction " KO " (parsed is nullptr)\n")
        rollback_transaction();
        return nullptr;
    }

    InstructionNode *instr_node = m_graph->create_instr();

    if (m_token_ribbon.canEat())
    {
        std::shared_ptr<Token> expectedEOI = m_token_ribbon.eatToken(Token_t::end_of_instruction);
        if (expectedEOI)
        {
            instr_node->end_of_instr_token(expectedEOI);
        } else if (m_token_ribbon.peekToken()->m_type != Token_t::fct_params_end)
        {
            LOG_VERBOSE("Parser", "parse instruction " KO " (end of instruction not found)\n")
            rollback_transaction();
            return nullptr;
        }
    }

    m_graph->connect(expression->get_owner(), instr_node);

    LOG_VERBOSE("Parser", "parse instruction " OK "\n")
    commit_transaction();
    return instr_node;
}

Node *Nodlang::parse_program()
{
    start_transaction();

    m_graph->clear();
    Node *root = m_graph->create_root();
    Scope *program_scope = root->get<Scope>();
    m_scope_stack.push(program_scope);

    parse_code_block(false);// we do not check if we parsed something empty or not, a program can be empty.

    // Add ignored chars pre/post token to the main scope begin/end token prefix/suffix.
    NDBL_ASSERT(!program_scope->get_begin_scope_token())
    NDBL_ASSERT(!program_scope->get_end_scope_token())
    program_scope->set_begin_scope_token(m_token_ribbon.m_prefix_acc);
    program_scope->set_end_scope_token(m_token_ribbon.m_suffix_acc);

    m_scope_stack.pop();
    commit_transaction();

    return m_graph->get_root();
}

Node *Nodlang::parse_scope()
{
    Node *result;

    start_transaction();

    if (!m_token_ribbon.eatToken(Token_t::scope_begin))
    {
        rollback_transaction();
        result = nullptr;
    } else
    {
        auto scope_node = m_graph->create_scope();
        auto scope = scope_node->get<Scope>();
        /*
         * link scope with parent_scope.
         * They must be linked in order to find_variables recursively.
         */
        auto parent_scope = m_scope_stack.top();
        if (parent_scope)
        {
            m_graph->connect({scope_node, Edge_t::IS_CHILD_OF, parent_scope->get_owner()});
        }

        m_scope_stack.push(scope);

        scope->set_begin_scope_token(m_token_ribbon.getEaten());

        parse_code_block(false);

        if (!m_token_ribbon.eatToken(Token_t::scope_end))
        {
            m_graph->destroy(scope_node);
            rollback_transaction();
            result = nullptr;
        } else
        {
            scope->set_end_scope_token(m_token_ribbon.getEaten());
            commit_transaction();
            result = scope_node;
        }

        m_scope_stack.pop();
    }
    return result;
}

IScope *Nodlang::parse_code_block(bool _create_scope)
{
    start_transaction();

    auto curr_scope = _create_scope ? m_graph->create_scope()->get<Scope>() : get_current_scope();

    NDBL_ASSERT(curr_scope);// needed

    bool stop = false;

    while (m_token_ribbon.canEat() && !stop)
    {
        if (InstructionNode *instr_node = parse_instr())
        {
            m_graph->connect({instr_node, Edge_t::IS_CHILD_OF, m_scope_stack.top()->get_owner()});
        } else if (parse_conditional_structure())
        {
        } else if (parse_for_loop())
        {
        } else if (parse_scope())
        {
        } else
        {
            stop = true;
        }
    }

    if (curr_scope->get_owner()->children_slots().empty())
    {
        rollback_transaction();
        return nullptr;
    } else
    {
        commit_transaction();
        return curr_scope;
    }
}

Property *Nodlang::parse_expression(unsigned short _precedence, Property *_leftOverride)
{
    LOG_VERBOSE("Parser", "parse expr...\n")
    LOG_VERBOSE("Parser", "%s \n", m_token_ribbon.toString().c_str())

    if (!m_token_ribbon.canEat())
    {
        LOG_VERBOSE("Parser", "parse expr..." KO " (unable to eat a single token)\n")
        return _leftOverride;
    }

    /*
		Get the left handed operand
	*/
    Property *left = _leftOverride;
    if (left == nullptr) left = parse_parenthesis_expression();
    if (left == nullptr) left = parse_unary_operator_expression(_precedence);
    if (left == nullptr) left = parse_function_call();
    if (left == nullptr) left = parse_variable_declaration();
    if (left == nullptr) left = parse_atomic_expression();

    if (!m_token_ribbon.canEat())
    {
        LOG_VERBOSE("Parser", "parse expr... " OK " (last token reached)\n")
    }

    Property *result;

    /*
		Get the right handed operand
	*/
    if (left)
    {
        LOG_VERBOSE("Parser", "parse expr... left parsed, we parse right\n")
        auto binResult = parse_binary_operator_expression(_precedence, left);

        if (binResult)
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
    auto currTokIt = m_token_ribbon.tokens.begin();
    short int opened = 0;

    while (currTokIt != m_token_ribbon.tokens.end() && success)
    {
        switch ((*currTokIt)->m_type)
        {
            case Token_t::fct_params_begin:
            {
                opened++;
                break;
            }
            case Token_t::fct_params_end:
            {
                if (opened <= 0)
                {
                    LOG_ERROR("Parser", "Syntax Error: Unexpected close bracket after \"... %s\" (position %llu)\n", m_token_ribbon.concat_token_buffers((*currTokIt)->m_index, -10).c_str(), (*currTokIt)->m_charIndex)
                    success = false;
                }
                opened--;
                break;
            }
            default:
                break;
        }

        std::advance(currTokIt, 1);
    }

    if (opened > 0)// same opened/closed parenthesis count required.
    {
        LOG_ERROR("Parser", "Syntax Error: Bracket count mismatch, %i still opened.\n", opened)
        success = false;
    }

    return success;
}

bool Nodlang::tokenize(const std::string &_string)
{
    std::string::const_iterator cursor = _string.cbegin();// the current char
    std::string pending_ignored_chars;

    /* shortcuts to language propertys */
    auto &regex = get_token_regexes();
    auto &regexIdToTokType = get_token_t_by_regex_index();

    // Parsing method #1: loop over all regex (might be slow).
    auto parse_token_using_regexes = [&]() -> std::shared_ptr<Token> {
        std::shared_ptr<Token> result;
        size_t index = std::distance(_string.cbegin(), cursor);

        for (auto &&each_regex_it = regex.cbegin(); each_regex_it != regex.cend(); each_regex_it++)
        {
            std::smatch sm;
            auto match = std::regex_search(cursor, _string.cend(), sm, *each_regex_it, std::regex_constants::match_continuous);

            if (match)
            {
                std::string matched_str = sm.str();
                Token_t matched_token_t = regexIdToTokType[std::distance(regex.cbegin(), each_regex_it)];
                result = std::make_shared<Token>(matched_token_t, matched_str, index);
                std::advance(cursor, matched_str.length());

                LOG_VERBOSE("Parser", "Token identified as %s at char %i: %s\n", to_string(matched_token_t).c_str(), index, matched_str.c_str())

                return result;
            }
        }
        return result;
    };

    // Parsing method #2: should be faster (that's the objective)
    auto parse_token_fast = [&]() -> std::shared_ptr<Token> {
        std::shared_ptr<Token> result;
        size_t cursor_idx = std::distance(_string.cbegin(), cursor);
        size_t char_left = _string.size() - cursor_idx;
        std::string::value_type current_char = _string.data()[cursor_idx];

        // single char
        switch (current_char)
        {
            case '{':
                cursor++;
                return std::make_shared<Token>(Token_t::scope_begin, current_char, cursor_idx);
            case '}':
                cursor++;
                return std::make_shared<Token>(Token_t::scope_end, current_char, cursor_idx);
            case '(':
                cursor++;
                return std::make_shared<Token>(Token_t::fct_params_begin, current_char, cursor_idx);
            case ')':
                cursor++;
                return std::make_shared<Token>(Token_t::fct_params_end, current_char, cursor_idx);
            case ',':
                cursor++;
                return std::make_shared<Token>(Token_t::fct_params_separator, current_char, cursor_idx);
            case ';':
                cursor++;
                return std::make_shared<Token>(Token_t::end_of_instruction, current_char, cursor_idx);
            case '\n':// fallthrough ...
            case ' ': // fallthrough ...
            case '\t':
                cursor++;
                return std::make_shared<Token>(Token_t::ignore, current_char, cursor_idx);
            default: /* no warning */;
        }

        // booleans
        if (_string.compare(cursor_idx, 4, "true") == 0)
        {
            result = std::make_shared<Token>(Token_t::literal_bool, "true", cursor_idx);
            std::advance(cursor, 4);
            return result;
        }
        if (_string.compare(cursor_idx, 5, "false") == 0)
        {
            result = std::make_shared<Token>(Token_t::literal_bool, "false", cursor_idx);
            std::advance(cursor, 5);
            return result;
        }

        // numbers
        //        if( *cursor >= '0' && *cursor <= '9' )
        //        {
        //            auto temp_cursor = cursor;
        //            while( temp_cursor!=_string.cend() && *temp_cursor >= '0' && *temp_cursor <= '9' )
        //            {
        //                ++temp_cursor;
        //            }
        //            if()
        //            result = std::make_shared<Token>(Token_t::literal_int, "true", cursor_idx);
        //        }
        return nullptr;
    };

    while (cursor != _string.cend())
    {
        std::shared_ptr<Token> new_token;

        // first, we try to tokenize using a WIP technique not involving any regex
        new_token = parse_token_fast();

        // then, if nothing matches, we try using our old technique using regexes
        if (!new_token)
        {
            new_token = parse_token_using_regexes();
        }

        if (!new_token)
        {
            size_t distance = std::distance(_string.cbegin(), cursor);
            LOG_WARNING("Parser", "Scanner Error: unable to tokenize %s at index %llu\n", _string.substr(distance, 10).c_str(), distance)
            return false;
        }

        // Finally we push the token in the ribbon
        if (new_token->m_type != Token_t::ignore)
        {
            /*
             * Append ignored_chars, 3 options:
             */
            if (!pending_ignored_chars.empty())
            {
                if (!m_token_ribbon.empty())
                {
                    std::shared_ptr<Token> last_token = m_token_ribbon.tokens.back();
                    if (last_token->m_type != Token_t::identifier)
                    {
                        /*
                        * Option 1: suffix of previous token
                        *
                        * ( ... , <last_token><pending_ignored_chars>, <new-token>)
                        */
                        last_token->append_to_suffix(pending_ignored_chars);
                        pending_ignored_chars.clear();
                    } else
                    {
                        /*
                        * Option 2: prefix of next/new token
                        *
                        * ( ... , <last_token>, <pending_ignored_chars><new-token>)
                        */
                        new_token->append_to_prefix(pending_ignored_chars);
                        pending_ignored_chars.clear();
                    }
                } else
                {
                    /*
                    * Option 3: prefix of the ribbon
                    *
                    * <pending_ignored_chars> (<first-and-new-token>)
                    */
                    m_token_ribbon.m_prefix_acc->append_to_word(pending_ignored_chars);
                    pending_ignored_chars.clear();
                }
            }
            m_token_ribbon.push(new_token);
            LOG_VERBOSE("Parser", "Append: %s\n", Token::to_string(new_token).c_str())
        } else
        {
            /*
             * When a character is ignored, it goes to a pending_chars string.
             * Once a no ignored token is parsed those pending_chars are added to the suffix (option 1) or to the prefix (option 2)
             */
            pending_ignored_chars.append(new_token->get_word());
            LOG_VERBOSE("Parser", "Append ignored: %s\n", Token::to_string(new_token).c_str())
        }
    }

    /*
	 * Append pending ignored chars
	 */
    if (!pending_ignored_chars.empty())
    {
        m_token_ribbon.m_suffix_acc->append_to_word(pending_ignored_chars);
        pending_ignored_chars.clear();
    }
    return true;
}

Property *Nodlang::parse_function_call()
{
    LOG_VERBOSE("Parser", "parse function call...\n")

    // Check if the minimum token count required is available ( 0: identifier, 1: open parenthesis, 2: close parenthesis)
    if (!m_token_ribbon.canEat(3))
    {
        LOG_VERBOSE("Parser", "parse function call... " KO " aborted, not enough tokens.\n")
        return nullptr;
    }

    start_transaction();

    // Try to parse regular function: function(...)
    std::string fct_id;
    std::shared_ptr<Token> token_0 = m_token_ribbon.eatToken();
    std::shared_ptr<Token> token_1 = m_token_ribbon.eatToken();
    if (token_0->m_type == Token_t::identifier &&
        token_1->m_type == Token_t::fct_params_begin)
    {
        fct_id = token_0->get_word();
        LOG_VERBOSE("Parser", "parse function call... " OK " regular function pattern detected.\n")
    } else// Try to parse operator like (ex: operator==(..,..))
    {
        std::shared_ptr<Token> token_2 = m_token_ribbon.eatToken();// eat a "supposed open bracket>

        if (token_0->m_type == Token_t::keyword_operator && token_1->m_type == Token_t::operator_ && token_2->m_type == Token_t::fct_params_begin)
        {
            fct_id = token_1->get_word();// operator
            LOG_VERBOSE("Parser", "parse function call... " OK " operator function-like pattern detected.\n")
        } else
        {
            LOG_VERBOSE("Parser", "parse function call... " KO " abort, this is not a function.\n")
            rollback_transaction();
            return nullptr;
        }
    }
    std::vector<Property *> args;

    // Declare a new function prototype
    func_type signature(fct_id);
    signature.set_return_type(type::any);

    bool parsingError = false;
    while (!parsingError && m_token_ribbon.canEat() && m_token_ribbon.peekToken()->m_type != Token_t::fct_params_end)
    {

        if (auto property = parse_expression())
        {
            args.push_back(property);                // store argument as property (already parsed)
            signature.push_arg(property->get_type());// add a new argument type to the proto.
            m_token_ribbon.eatToken(Token_t::fct_params_separator);
        } else
        {
            parsingError = true;
        }
    }

    // eat "close bracket supposed" token
    if (!m_token_ribbon.eatToken(Token_t::fct_params_end))
    {
        LOG_WARNING("Parser", "parse function call... " KO " abort, close parenthesis expected. \n")
        rollback_transaction();
        return nullptr;
    }


    // Find the prototype in the language library
    std::shared_ptr<const iinvokable> invokable = find_function(&signature);

    auto connectArg = [&](const func_type *_sig, Node *_node, size_t _arg_index) -> void {// lambda to connect input property to node for a specific argument index.
        Property *src_property = args.at(_arg_index);
        Property *dst_property = _node->props()->get_input_at(_arg_index);
        NDBL_ASSERT(dst_property)
        m_graph->connect(src_property, dst_property);
    };

    Node *node;
    if (invokable)
    {
        /*
         * If we found a function matching signature, we create a node with that function.
         * The node will be able to be evaluated.
         *
         * TODO: remove this method, the parser should not check if function exist or not.
         *       this role is for the Compiler.
         */
        node = m_graph->create_function(invokable.get());
    } else
    {
        /*
         * If we DO NOT found a function matching signature, we create an abstract function.
         * The node will be able to be evaluated.
         */
        node = m_graph->create_abstract_function(&signature);
    }

    for (size_t argIndex = 0; argIndex < signature.get_arg_count(); argIndex++)
    {
        connectArg(&signature, node, argIndex);
    }

    commit_transaction();
    LOG_VERBOSE("Parser", "parse function call... " OK "\n")

    return node->props()->get(k_value_property_name);
}

Scope *Nodlang::get_current_scope()
{
    NDBL_ASSERT(m_scope_stack.top());// stack SHALL not be empty.
    return m_scope_stack.top();
}

ConditionalStructNode *Nodlang::parse_conditional_structure()
{
    LOG_VERBOSE("Parser", "try to parse conditional structure...\n")
    start_transaction();

    bool success = false;
    ConditionalStructNode *condStruct = m_graph->create_cond_struct();

    if (m_token_ribbon.eatToken(Token_t::keyword_if))
    {
        m_graph->connect({condStruct, Edge_t::IS_CHILD_OF, m_scope_stack.top()->get_owner()});
        m_scope_stack.push(condStruct->get<Scope>());

        condStruct->set_token_if(m_token_ribbon.getEaten());

        if (m_token_ribbon.eatToken(Token_t::fct_params_begin))
        {
            InstructionNode *condition = parse_instr();

            if (condition)
            {
                condition->set_name("Condition");
                condition->set_name("Cond.");
                condStruct->set_cond_expr(condition);

                if (m_token_ribbon.eatToken(Token_t::fct_params_end))
                {
                    m_graph->connect(condition->get_this_property(), condStruct->condition_property());

                    if (Node *scopeIf = parse_scope())
                    {
                        if (m_token_ribbon.eatToken(Token_t::keyword_else))
                        {
                            condStruct->set_token_else(m_token_ribbon.getEaten());

                            /* parse else scope */
                            if (parse_scope())
                            {
                                LOG_VERBOSE("Parser", "parse IF {...} ELSE {...} block... " OK "\n")
                                success = true;
                            }
                            /* (or) parse else if scope */
                            else if (ConditionalStructNode *else_cond_struct = parse_conditional_structure())
                            {
                                else_cond_struct->set_name("else if");

                                LOG_VERBOSE("Parser", "parse IF {...} ELSE IF {...} block... " OK "\n")
                                success = true;
                            } else
                            {
                                LOG_VERBOSE("Parser", "parse IF {...} ELSE {...} block... " KO "\n")
                                m_graph->destroy(scopeIf);
                            }
                        } else
                        {
                            LOG_VERBOSE("Parser", "parse IF {...} block... " OK "\n")
                            success = true;
                        }
                    } else
                    {
                        if (condition)
                        {
                            m_graph->destroy(condition);
                        }
                        LOG_VERBOSE("Parser", "parse IF {...} block... " KO "\n")
                    }
                } else
                {
                    LOG_VERBOSE("Parser", "parse IF (...) <--- close bracket missing { ... }  " KO "\n")
                    success = false;
                }
            }
        }
        m_scope_stack.pop();
    }

    if (success)
    {
        commit_transaction();
    } else
    {
        m_graph->destroy(condStruct);
        condStruct = nullptr;
        rollback_transaction();
    }

    return condStruct;
}

ForLoopNode *Nodlang::parse_for_loop()
{
    bool success = false;
    ForLoopNode *for_loop_node = nullptr;
    start_transaction();

    std::shared_ptr<Token> token_for = m_token_ribbon.eatToken(Token_t::keyword_for);

    if (token_for != nullptr)
    {
        for_loop_node = m_graph->create_for_loop();
        m_graph->connect({for_loop_node, Edge_t::IS_CHILD_OF, m_scope_stack.top()->get_owner()});
        m_scope_stack.push(for_loop_node->get<Scope>());

        for_loop_node->set_token_for(token_for);

        LOG_VERBOSE("Parser", "parse FOR (...) block...\n")
        std::shared_ptr<Token> open_bracket = m_token_ribbon.eatToken(Token_t::fct_params_begin);
        if (!open_bracket)
        {
            LOG_ERROR("Parser", "Unable to find open bracket after for keyword.\n")
        } else
        {
            InstructionNode *init_instr = parse_instr();
            if (!init_instr)
            {
                LOG_ERROR("Parser", "Unable to find initial instruction.\n")
            } else
            {
                init_instr->set_name("Initialisation");
                m_graph->connect(init_instr->get_this_property(), for_loop_node->get_init_expr());
                for_loop_node->set_init_instr(init_instr);

                InstructionNode *cond_instr = parse_instr();
                if (!cond_instr)
                {
                    LOG_ERROR("Parser", "Unable to find condition instruction.\n")
                } else
                {
                    cond_instr->set_name("Condition");
                    m_graph->connect(cond_instr->get_this_property(), for_loop_node->condition_property());
                    for_loop_node->set_cond_expr(cond_instr);

                    InstructionNode *iter_instr = parse_instr();
                    if (!iter_instr)
                    {
                        LOG_ERROR("Parser", "Unable to find iterative instruction.\n")
                    } else
                    {
                        iter_instr->set_name("Iteration");
                        m_graph->connect(iter_instr->get_this_property(), for_loop_node->get_iter_expr());
                        for_loop_node->set_iter_instr(iter_instr);

                        std::shared_ptr<Token> close_bracket = m_token_ribbon.eatToken(Token_t::fct_params_end);
                        if (!close_bracket)
                        {
                            LOG_ERROR("Parser", "Unable to find close bracket after iterative instruction.\n")
                        } else if (!parse_scope())
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
        m_scope_stack.pop();
    }

    if (success)
    {
        commit_transaction();
    } else
    {
        rollback_transaction();
        if (for_loop_node) m_graph->destroy(for_loop_node);
        for_loop_node = nullptr;
    }

    return for_loop_node;
}

Property *Nodlang::parse_variable_declaration()
{

    if (!m_token_ribbon.canEat(2))
        return nullptr;

    start_transaction();

    std::shared_ptr<Token> tok_type = m_token_ribbon.eatToken();
    std::shared_ptr<Token> tok_identifier = m_token_ribbon.eatToken();

    if (tok_type->is_keyword_type() && tok_identifier->m_type == Token_t::identifier)
    {
        type type = get_type(tok_type->m_type);
        VariableNode *variable = m_graph->create_variable(type, tok_identifier->get_word(), get_current_scope());
        variable->set_declared(true);
        variable->set_type_token(tok_type);
        variable->get_identifier_token()->transfer_prefix_suffix(tok_identifier);
        variable->get_value()->set_src_token(std::make_shared<Token>(*tok_identifier));

        // try to parse assignment
        std::shared_ptr<Token> assignmentTok = m_token_ribbon.eatToken(Token_t::operator_);
        if (assignmentTok && assignmentTok->get_word() == "=")
        {
            auto expression_result = parse_expression();
            if (expression_result &&
                type::is_implicitly_convertible(expression_result->get_type(), variable->get_value()->get_type()))
            {
                m_graph->connect(expression_result, variable);
                variable->set_assignment_operator_token(assignmentTok);
            } else
            {
                LOG_ERROR("Parser", "Unable to parse expression to assign %s\n", tok_identifier->get_word().c_str())
                rollback_transaction();
                m_graph->destroy(variable);
                return nullptr;
            }
        }

        commit_transaction();
        return variable->get_value();
    }

    rollback_transaction();
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------------
// [SECTION] C. Serializer --------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------

std::string &Nodlang::serialize(std::string &_out, const InvokableComponent *_component) const
{
    const func_type *type = _component->get_func_type();

    if (_component->is_operator())
    {
        // generic serialize property lambda
        auto serialize_property_with_or_without_brackets = [this, &_out](Property *property, bool needs_brackets) {
            if (needs_brackets)
            {
                serialize(_out, Token_t::fct_params_begin);
            }

            serialize(_out, property);

            if (needs_brackets)
            {
                serialize(_out, Token_t::fct_params_end);
            }
        };

        Node *owner = _component->get_owner();
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
                std::shared_ptr<Token> sourceToken = _component->get_source_token();
                if (sourceToken)
                {
                    _out.append(sourceToken->m_buffer);
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
                std::shared_ptr<Token> token = _component->get_source_token();

                if (token) _out.append(token->get_prefix());

                _out.append(type->get_identifier());

                if (token) _out.append(token->get_suffix());

                auto inner_operator = owner->get_connected_invokable(args[0]);
                serialize_property_with_or_without_brackets(args[0], inner_operator != nullptr);
                break;
            }
        }
    } else
    {
        serialize(_out, type, _component->get_args());
    }

    return _out;
}

std::string &Nodlang::serialize(std::string &_out, const func_type *_signature, const std::vector<Property *> &_args) const
{
    _out.append(_signature->get_identifier());
    serialize(_out, Token_t::fct_params_begin);

    for (auto it = _args.begin(); it != _args.end(); it++)
    {
        serialize(_out, *it);

        if (*it != _args.back())
        {
            serialize(_out, Token_t::fct_params_separator);
        }
    }

    serialize(_out, Token_t::fct_params_end);
    return _out;
}

std::string &Nodlang::serialize(std::string &_out, const func_type *_signature) const
{
    serialize(_out, _signature->get_return_type());
    _out.append(" ");
    _out.append(_signature->get_identifier());
    serialize(_out, Token_t::fct_params_begin);

    auto args = _signature->get_args();
    for (auto it = args.begin(); it != args.end(); it++)
    {
        if (it != args.begin())
        {
            serialize(_out, Token_t::fct_params_separator);
            _out.append(" ");
        }
        serialize(_out, it->m_type);
    }

    serialize(_out, Token_t::fct_params_end);
    return _out;
}

std::string &Nodlang::serialize(std::string &_out, const Token_t &_type) const
{
    return _out.append(to_string(_type));
}

std::string &Nodlang::serialize(std::string &_out, type _type) const
{
    return _out.append(to_string(_type));
}

std::string &Nodlang::serialize(std::string &_out, const VariableNode *_node) const
{
    const InstructionNode *decl_instr = _node->get_declaration_instr();

    // 1. Serialize variable's type

    if (decl_instr)
    {
        // If parsed
        if (std::shared_ptr<const Token> type_tok = _node->get_type_token())
        {
            serialize(_out, type_tok);
        }
        else // If created in the graph by the user
        {
            serialize(_out, _node->get_value()->get_type());
            _out.append(" ");
        }
    }

    // 2. Serialize variable identifier

    _out.append(_node->get_identifier_token()->m_buffer);

    // 3. If variable is connected, serialize its assigned expression

    Property *value = _node->get_value();
    if (decl_instr && value->has_input_connected())
    {
        auto append_assign_tok = [&]() {
            std::shared_ptr<const Token> assign_tok = _node->get_assignment_operator_token();
            _out.append(assign_tok ? assign_tok->m_buffer : " = ");
        };

        append_assign_tok();
        serialize(_out, value);
    }
    return _out;
}

std::string &Nodlang::serialize(std::string &_out, const variant *variant) const
{
    std::string variant_string = variant->convert_to<std::string>();

    if (variant->get_type() == type::get<std::string>())
    {
        return _out.append('"' + variant_string + '"');
    }
    return _out.append(variant_string);
}

std::string &Nodlang::serialize(std::string &_out, const Property *_property, bool recursively) const
{
    // specific case of a Node*
    if (_property->get_type() == type::get<Node *>())
    {
        if (_property->get_variant()->is_initialized())
        {
            return serialize(_out, (const Node *) *_property);
        }
    }

    std::shared_ptr<Token> sourceToken = _property->get_src_token();
    if (sourceToken)
    {
        _out.append(sourceToken->get_prefix());
    }

    auto owner = _property->get_owner();
    if (recursively && owner && _property->allows_connection(Way_In) && owner->is_connected_with(_property))
    {
        Property *src_property = _property->get_input();
        InvokableComponent *compute_component = src_property->get_owner()->get<InvokableComponent>();

        if (compute_component)
        {
            serialize(_out, compute_component);
        } else
        {
            serialize(_out, src_property, false);
        }
    } else
    {
        if (owner && owner->get_type() == type::get<VariableNode>())
        {
            _out.append(owner->as<VariableNode>()->get_name());
        } else
        {
            serialize(_out, _property->get_variant());
        }
    }

    if (sourceToken)
    {
        _out.append(sourceToken->get_suffix());
    }
    return _out;
}

std::string &Nodlang::serialize(std::string &_out, const Node *_node) const
{
    NDBL_ASSERT(_node != nullptr)
    type type = _node->get_type();

    if (type.is_child_of<InstructionNode>())
    {
        serialize(_out, _node->as<InstructionNode>());
    } else if (type.is_child_of<ConditionalStructNode>())
    {
        serialize(_out, _node->as<ConditionalStructNode>());
    } else if (type.is_child_of<ForLoopNode>())
    {
        serialize(_out, _node->as<ForLoopNode>());
    } else if (_node->has<Scope>())
    {
        serialize(_out, _node->get<Scope>());
    } else if (_node->is<LiteralNode>())
    {
        serialize(_out, _node->as<LiteralNode>()->get_value());
    } else if (_node->is<VariableNode>())
    {
        serialize(_out, _node->as<VariableNode>());
    } else if (_node->has<InvokableComponent>())
    {
        serialize(_out, _node->get<InvokableComponent>());
    } else
    {
        std::string message = "Unable to serialize ";
        message.append(type.get_name());
        throw std::runtime_error(message);
    }

    return _out;
}

std::string &Nodlang::serialize(std::string &_out, const Scope *_scope) const
{

    serialize(_out, _scope->get_begin_scope_token());
    auto &children = _scope->get_owner()->children_slots();
    if (!children.empty())
    {
        for (auto &eachChild: children)
        {
            serialize(_out, eachChild);
        }
    }

    serialize(_out, _scope->get_end_scope_token());

    return _out;
}

std::string &Nodlang::serialize(std::string &_out, const InstructionNode *_instruction) const
{
    const Property *root_node_property = _instruction->get_root_node_property();

    if (root_node_property->has_input_connected() && root_node_property->get_variant()->is_initialized())
    {
        auto root_node = (const Node *) *root_node_property;
        NDBL_ASSERT(root_node)
        serialize(_out, root_node);
    }

    return serialize(_out, _instruction->end_of_instr_token());
}

std::string &Nodlang::serialize(std::string &_out, std::shared_ptr<const Token> _token) const
{
    if (_token)
    {
        _out.append(_token->get_prefix());
        if (_token->m_type == Token_t::unknown)
        {
            _out.append(_token->get_word());
        } else
        {
            serialize(_out, _token->m_type);
        }
        _out.append(_token->get_suffix());
    }
    return _out;
}

std::string &Nodlang::serialize(std::string &_out, const ForLoopNode *_for_loop) const
{

    serialize(_out, _for_loop->get_token_for());
    serialize(_out, Token_t::fct_params_begin);

    // TODO: I don't like this if/else, should be implicit. Serialize Property* must do it.
    //       More work to do to know if expression is a declaration or not.

    Property *input = _for_loop->get_init_expr()->get_input();
    if (input && input->get_owner()->get_type().is_child_of<VariableNode>())
    {
        serialize(_out, input->get_owner()->as<VariableNode>());
    } else
    {
        serialize(_out, _for_loop->get_init_instr());
    }
    serialize(_out, _for_loop->get_cond_expr());
    serialize(_out, _for_loop->get_iter_instr());
    serialize(_out, Token_t::fct_params_end);

    // if scope
    if (auto *scope = _for_loop->get_condition_true_scope())
    {
        serialize(_out, scope);
    }

    return _out;
}

std::string &Nodlang::serialize(std::string &_out, const ConditionalStructNode *_condStruct) const
{
    // if ( <condition> )
    serialize(_out, _condStruct->get_token_if());
    serialize(_out, Token_t::fct_params_begin);
    serialize(_out, _condStruct->get_cond_expr());
    serialize(_out, Token_t::fct_params_end);

    // if scope
    if (auto *ifScope = _condStruct->get_condition_true_scope())
        serialize(_out, ifScope);

    // else & else scope
    if (std::shared_ptr<const Token> tokenElse = _condStruct->get_token_else())
    {
        serialize(_out, tokenElse);
        Scope *elseScope = _condStruct->get_condition_false_scope();
        if (elseScope)
        {
            serialize(_out, elseScope->get_owner());
        }
    }
    return _out;
}

// Language definition ------------------------------------------------------------------------------------------------------------

std::shared_ptr<const iinvokable> Nodlang::find_function(const func_type *_signature) const
{
    auto is_compatible = [&](std::shared_ptr<const iinvokable> fct) {
        return fct->get_type().is_compatible(_signature);
    };

    auto it = std::find_if(m_functions.begin(), m_functions.end(), is_compatible);

    if (it != m_functions.end())
    {
        return *it;
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
        return _type->is_exactly(&_invokable->get_type());
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
        return _type->is_compatible(&_invokable->get_type());
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

    const func_type *type = &_invokable->get_type();

    std::string type_as_string;
    serialize(type_as_string, type);

    // Stops if no operator having the same identifier and argument count is found
    if (!find_operator(type->get_identifier(), static_cast<Operator_t>(type->get_arg_count())))
    {
        LOG_VERBOSE("Nodlang", "add function: %s (in m_functions)\n", type_as_string.c_str());
        return;
    }

    // Register the invokable as an operator implementation
    auto found = std::find(m_operators_impl.begin(), m_operators_impl.end(), _invokable);
    NDBL_ASSERT(found == m_operators_impl.end())
    m_operators_impl.push_back(_invokable);
    LOG_VERBOSE("Nodlang", "add operator: %s (in m_functions and m_operator_implems)\n", type_as_string.c_str());
}

void Nodlang::add_operator(const char *_id, Operator_t _type, int _precedence)
{
    const Operator *op = new Operator(_id, _type, _precedence);

    NDBL_ASSERT(std::find(m_operators.begin(), m_operators.end(), op) == m_operators.end())
    m_operators.push_back(op);
}

const Operator *Nodlang::find_operator(const std::string &_identifier, Operator_t _type) const
{
    auto is_exactly = [&](const Operator *op) {
        return op->identifier == _identifier && op->type == _type;
    };

    auto found = std::find_if(m_operators.cbegin(), m_operators.cend(), is_exactly);

    if (found != m_operators.end())
        return *found;

    return nullptr;
}


void Nodlang::add_regex(const std::regex &_regex, Token_t _token_t)
{
    m_token_regex.push_back(_regex);
    m_token_t_by_regex_index.push_back(_token_t);
}

void Nodlang::add_regex(const std::regex &_regex, Token_t _token_t, type _type)
{
    m_token_regex.push_back(_regex);
    m_token_t_by_regex_index.push_back(_token_t);

    m_type_regex.push_back(_regex);
    m_type_by_regex_index.push_back(_type);

    m_token_type_keyword_to_type.insert({_token_t, _type});
    m_type_hashcode_to_token_t.insert({_type.hash_code(), _token_t});
}

void Nodlang::add_type(type _type, Token_t _token_t, std::string _string)
{
    m_token_type_keyword_to_type.insert({_token_t, _type});
    m_type_hashcode_to_token_t.insert({_type.hash_code(), _token_t});
    m_type_hashcode_to_string.insert({_type.hash_code(), _string});
    add_string(_string, _token_t);
}

void Nodlang::add_string(std::string _string, Token_t _token_t)
{
    m_token_t_to_string.insert({_token_t, _string});
    add_regex(std::regex("^(" + _string + ")"), _token_t);
}

void Nodlang::add_char(const char _char, Token_t _token_t)
{
    m_token_t_to_char.insert({_token_t, _char});
    m_char_to_token_t.insert({_char, _token_t});
}

std::string &Nodlang::to_string(std::string &_out, const type& _type) const
{
    auto found = m_type_hashcode_to_string.find(_type.hash_code());
    if (found != m_type_hashcode_to_string.cend())
    {
        return _out.append(found->second);
    }
    return _out;
}

std::string &Nodlang::to_string(std::string &_out, Token_t _token_t) const
{
    switch (_token_t)
    {
        case Token_t::ignore:
            return _out.append("ignore");
        case Token_t::operator_:
            return _out.append("operator");
        case Token_t::identifier:
            return _out.append("identifier");
    }

    {
        auto found = m_token_t_to_char.find(_token_t);
        if (found != m_token_t_to_char.cend())
        {
            _out.push_back(found->second);
            return _out;
        }
    }
    auto found = m_token_t_to_string.find(_token_t);
    if (found != m_token_t_to_string.cend())
    {
        return _out.append(found->second);
    }

    return _out.append("<?>");
}

std::string Nodlang::to_string(type _type) const
{
    std::string result;
    return to_string(result, _type);
}

std::string Nodlang::to_string(Token_t _token) const
{
    std::string result;
    return to_string(result, _token);
}

int Nodlang::get_precedence(const iinvokable *_invokable) const
{
    if (!_invokable) return std::numeric_limits<int>::min();// default

    auto type = _invokable->get_type();
    auto oper = find_operator(type.get_identifier(), static_cast<Operator_t>(type.get_arg_count()));

    if (!oper) return 0;// default

    return oper->precedence;
}

type Nodlang::get_type(Token_t _token) const
{
    NDBL_EXPECT(is_a_type_keyword(_token), "_token_t is not a type keyword!");
    return m_token_type_keyword_to_type.find(_token)->second;
}

Nodlang& Nodlang::get_instance()
{
    static Nodlang instance;
    return instance;
}
