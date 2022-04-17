#include <nodable/core/Parser.h>

#include <regex>
#include <algorithm>
#include <sstream>
#include <string>
#include <memory> // std::shared_ptr

#include <nodable/core/Log.h>
#include <nodable/core/Member.h>
#include <nodable/core/Wire.h>
#include <nodable/core/GraphNode.h>
#include <nodable/core/InstructionNode.h>
#include <nodable/core/LiteralNode.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/InvokableComponent.h>
#include <nodable/core/Scope.h>
#include <nodable/core/System.h>

using namespace Nodable;

Parser::Parser(const Language* _lang, bool _strict )
        : m_language(*_lang)
        , m_strict_mode(_strict)
        , m_graph(nullptr){}

void Parser::rollback_transaction()
{
    m_token_ribbon.rollbackTransaction();
}

void Parser::start_transaction()
{
    m_token_ribbon.startTransaction();
}

void Parser::commit_transaction()
{
    m_token_ribbon.commitTransaction();
}

bool Parser::parse_graph(const std::string &_source_code, GraphNode *_graphNode)
{
    m_graph = _graphNode;
    m_token_ribbon.clear();

    LOG_VERBOSE("Parser", "Trying to evaluate evaluated: <expr>%s</expr>\"\n", _source_code.c_str() )

    std::istringstream iss(_source_code);
    std::string line;
    LOG_MESSAGE("Parser", "Tokenization ...\n" )
    size_t line_count = 0;
    while (std::getline(iss, line, System::k_end_of_line ))
    {
        LOG_VERBOSE("Parser", "Tokenization on line %llu ...\n", line_count )

        if (line_count != 0 && !m_token_ribbon.tokens.empty() )
        {
            std::shared_ptr<Token> lastToken = m_token_ribbon.tokens.back();
            lastToken->m_suffix.push_back(System::k_end_of_line);
        }

        if (!tokenize_string(line))
        {
            LOG_WARNING("Parser", "Unable to tokenize!\n")
            return false;
        }
        LOG_VERBOSE("Parser", "Tokenization on line %llu OK\n", line_count )
        line_count++;
    }
    LOG_MESSAGE("Parser", "Tokenization OK (%llu line(s))\n", line_count )

	if (!is_syntax_valid())
	{
		LOG_WARNING("Parser", "Unable to parse code due to syntax error.\n")
		return false;
	}

	Node* program = parse_program();

	if (program == nullptr)
	{
		LOG_WARNING("Parser", "Unable to generate program tree.\n")
		return false;
	}

    if ( m_token_ribbon.canEat() )
    {
        m_graph->clear();
        LOG_WARNING("Parser", "Unable to generate a full program tree.\n")
        LOG_MESSAGE("Parser", "--- Token Ribbon begin ---\n");
        for( auto each_token : m_token_ribbon.tokens )
        {
            LOG_MESSAGE("Parser", "%i: %s\n", each_token->m_index, Token::to_string(each_token).c_str() );
        }
        LOG_MESSAGE("Parser", "--- Token Ribbon end ---\n");
        LOG_WARNING("Parser", "Stuck at token %i (charIndex %i).\n", (int)m_token_ribbon.get_curr_tok_idx(), (int)m_token_ribbon.peekToken()->m_charIndex )
        return false;
    }

    // We unset dirty, since we did a lot of connections but we don't want any update now
    auto& nodes = m_graph->get_node_registry();
    for(auto eachNode : nodes )
        eachNode->set_dirty(false);

	LOG_MESSAGE("Parser", "Program tree updated.\n", _source_code.c_str() )
	LOG_VERBOSE("Parser", "Source code: <expr>%s</expr>\"\n", _source_code.c_str() )
	return true;
}

type Parser::get_literal_type(std::shared_ptr<const Token>_token) const
{
    const Semantic& semantic                 = m_language.get_semantic();
    const std::vector<std::regex>  regex     = semantic.get_type_regex();
    const std::vector<type> regex_id_to_type = semantic.get_type_regex_index_to_type();

    type type = type::any;

    auto each_regex_it = regex.cbegin();
    while( each_regex_it != regex.cend() && type == type::any )
    {
        std::smatch sm;
        auto match = std::regex_search(_token->m_word.cbegin(), _token->m_word.cend(), sm, *each_regex_it);

        if (match)
        {
            size_t index = std::distance(regex.cbegin(), each_regex_it);
            type = regex_id_to_type[index];
        }
        each_regex_it++;
    }

    NODABLE_ASSERT(type != type::any)

    return type;
}

bool Parser::parse_bool(const std::string &_str)
{
    return _str == std::string("true");
}

std::string Parser::parse_string(const std::string &_str)
{
    NODABLE_ASSERT(_str.size() >= 2);
    NODABLE_ASSERT(_str.front() == '\"');
    NODABLE_ASSERT(_str.back() == '\"');
    return std::string(++_str.cbegin(), --_str.cend());
}

double Parser::parse_double(const std::string &_str)
{
    return stod(_str);
}

i16_t Parser::parse_int16(const std::string &_str)
{
    return stoi(_str);
}

Member* Parser::token_to_member(std::shared_ptr<Token> _token)
{
	Member* result;

	switch (_token->m_type)
	{

	    case Token_t::literal:
        {
            type type = get_literal_type(_token);
            LiteralNode* literal = m_graph->create_literal(type);

                 if( type == type::get<std::string>() ) literal->set_value(parse_string(_token->m_word) );
            else if( type == type::get<i16_t>() )       literal->set_value(parse_int16(_token->m_word) );
            else if( type == type::get<double>() )      literal->set_value(parse_double(_token->m_word) );
            else if( type == type::get<bool>() )        literal->set_value(parse_bool(_token->m_word)  );

            result = literal->get_value();
            result->set_src_token(_token);
            break;
        }

	    case Token_t::identifier:
		{
			VariableNode* variable = get_current_scope()->find_variable(_token->m_word);

			if (variable == nullptr)
			{
			    if ( m_strict_mode )
                {
                    LOG_ERROR("Parser", "Expecting declaration for symbol %s (strict mode) \n", _token->m_word.c_str())
                }
			    else
                {
			        /* when strict mode is OFF, we just create a variable with Any type */
                    LOG_WARNING("Parser", "Expecting declaration for symbol %s, compilation will fail.\n", _token->m_word.c_str())
                    variable = m_graph->create_variable(type::null, _token->m_word, get_current_scope());
                    variable->get_value()->set_src_token(_token);
                    variable->set_declared(false);
                }
            }
            if (variable == nullptr)
            {
                result = nullptr;
            }
            else
            {
                result = variable->get_value();
            }

			break;
		}

	    default:
            LOG_VERBOSE("Parser", "Unable to perform token_to_member for token %s!\n", _token->m_word.c_str())
            result = nullptr;
    }

	return result;
}

Member* Parser::parse_binary_operator_expression(unsigned short _precedence, Member *_left) {

    assert(_left != nullptr);

    LOG_VERBOSE("Parser", "parse binary operation expr...\n")
    LOG_VERBOSE("Parser", "%s \n", m_token_ribbon.toString().c_str())

	Member* result = nullptr;

	if ( !m_token_ribbon.canEat(2))
	{
		LOG_VERBOSE("Parser", "parse binary operation expr...... " KO " (not enought tokens)\n")
		return nullptr;
	}

    start_transaction();
    std::shared_ptr<Token> operatorToken = m_token_ribbon.eatToken();
    std::shared_ptr<Token> operandToken  = m_token_ribbon.peekToken();

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

	const Operator* ope = m_language.find_operator(operatorToken->m_word, Operator_t::Binary);
    if ( ope == nullptr ) {
        LOG_VERBOSE("Parser", "parse binary operation expr... " KO " (unable to find operator %s)\n", operatorToken->m_word.c_str())
        rollback_transaction();
        return nullptr;
    }

    // Precedence check
	if ( ope->precedence <= _precedence && _precedence > 0) { // always update the first operation if they have the same precedence or less.
		LOG_VERBOSE("Parser", "parse binary operation expr... " KO " (Precedence)\n")
        rollback_transaction();
		return nullptr;
	}


	// Parse right expression
	auto right = parse_expression( ope->precedence, nullptr);

	if (!right)
	{
		LOG_VERBOSE("Parser", "parseBinaryOperationExpression... " KO " (right expression is nullptr)\n")
        rollback_transaction();
		return nullptr;
	}

	// Create a function signature according to ltype, rtype and operator word
	const Signature* signature = Signature::new_operator(type::any, ope, _left->get_type(), right->get_type());
    const IInvokable* invokable = m_language.find_operator_fct(signature);

    InvokableComponent* component;
    Node* binary_op;
    if ( invokable )
	{
	    // concrete operator
		binary_op = m_graph->create_function(invokable);
        component = binary_op->get<InvokableComponent>();
        delete signature;
    }
	else if(signature)
    {
	    // abstract operator
        binary_op = m_graph->create_abstract_function(signature);
        component = binary_op->get<InvokableComponent>();
    }
    else
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
    return binary_op->props()->get(k_value_member_name);
}

Member* Parser::parse_unary_operator_expression(unsigned short _precedence)
{
	LOG_VERBOSE("Parser", "parseUnaryOperationExpression...\n")
	LOG_VERBOSE("Parser", "%s \n", m_token_ribbon.toString().c_str())

	if (!m_token_ribbon.canEat(2) )
	{
		LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (not enough tokens)\n")
		return nullptr;
	}

    start_transaction();
    std::shared_ptr<Token> operatorToken = m_token_ribbon.eatToken();

	// Check if we get an operator first
	if (operatorToken->m_type != Token_t::operator_)
	{
        rollback_transaction();
		LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (operator not found)\n")
		return nullptr;
	}

	// Parse expression after the operator
	Member* value = parse_atomic_expression();

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

	const Operator*   ope       = m_language.find_operator(operatorToken->m_word, Operator_t::Unary );
	const Signature*  sig       = Signature::new_operator(type::any, ope, value->get_type());
	const IInvokable* invokable = m_language.find_operator_fct(sig);

    InvokableComponent* component;
	Node* node;

	if (invokable)
	{
        node = m_graph->create_function(invokable);
        delete sig;
	}
	else if(sig)
	{
        node = m_graph->create_abstract_function(sig);
	}
    else
    {
        LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (no signature found)\n")
        rollback_transaction();
        return nullptr;
    }

    component = node->get<InvokableComponent>();
    component->set_source_token(operatorToken);

    m_graph->connect(value, component->get_l_handed_val());
    Member* result = node->props()->get(k_value_member_name);

    LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " OK "\n")
    commit_transaction();

    return result;
}

Member* Parser::parse_atomic_expression()
{
	LOG_VERBOSE("Parser", "parse atomic expr... \n")

	if ( !m_token_ribbon.canEat() )
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

	auto result = token_to_member(token);

	if( result != nullptr)
    {
        commit_transaction();
        LOG_VERBOSE("Parser", "parse atomic expr... " OK "\n")
    }
	else
    {
        rollback_transaction();
        LOG_VERBOSE("Parser", "parse atomic expr... " KO " (result is nullptr)\n")
	}

	return result;
}

Member* Parser::parse_parenthesis_expression()
{
	LOG_VERBOSE("Parser", "parse parenthesis expr...\n")
	LOG_VERBOSE("Parser", "%s \n", m_token_ribbon.toString().c_str())

	if ( !m_token_ribbon.canEat() )
	{
		LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " no enough tokens.\n")
		return nullptr;
	}

    start_transaction();
    std::shared_ptr<Token> currentToken = m_token_ribbon.eatToken();
	if (currentToken->m_type != Token_t::open_bracket)
	{
		LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " open bracket not found.\n")
        rollback_transaction();
		return nullptr;
	}

    Member* result = parse_expression();
	if (result)
	{
        std::shared_ptr<Token> token = m_token_ribbon.eatToken();
		if (token->m_type != Token_t::close_bracket )
		{
			LOG_VERBOSE("Parser", "%s \n", m_token_ribbon.toString().c_str())
			LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " ( \")\" expected instead of %s )\n", token->m_word.c_str() )
            rollback_transaction();
		}
		else
        {
			LOG_VERBOSE("Parser", "parse parenthesis expr..." OK  "\n")
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

InstructionNode* Parser::parse_instr()
{
    start_transaction();

    Member* expression = parse_expression();

    if ( !expression )
    {
       LOG_VERBOSE("Parser", "parse instruction " KO " (parsed is nullptr)\n")
        rollback_transaction();
       return nullptr;
    }

    InstructionNode* instr_node = m_graph->create_instr();

    if ( m_token_ribbon.canEat() )
    {
        std::shared_ptr<Token> expectedEOI = m_token_ribbon.eatToken(Token_t::end_of_instruction);
        if ( expectedEOI )
        {
            instr_node->end_of_instr_token(expectedEOI);
        }
        else if( m_token_ribbon.peekToken()->m_type != Token_t::close_bracket )
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

Node* Parser::parse_program()
{
    start_transaction();

    m_graph->clear();
    Node*  root          = m_graph->create_root();
    Scope* program_scope = root->get<Scope>();
    m_scope_stack.push( program_scope );

    parse_code_block(false); // we do not check if we parsed something empty or not, a program can be empty.

    // Add ignored chars pre/post token to the main scope begin/end token prefix/suffix.
    NODABLE_ASSERT(!program_scope->get_begin_scope_token())
    NODABLE_ASSERT(!program_scope->get_end_scope_token())
    program_scope->set_begin_scope_token( m_token_ribbon.m_prefix );
    program_scope->set_end_scope_token( m_token_ribbon.m_suffix );

    m_scope_stack.pop();
    commit_transaction();

    return m_graph->get_root();
}

Node* Parser::parse_scope()
{
    Node* result;

    start_transaction();

    if ( !m_token_ribbon.eatToken(Token_t::begin_scope))
    {
        rollback_transaction();
        result = nullptr;
    }
    else
    {
        auto scope_node = m_graph->create_scope();
        auto scope      = scope_node->get<Scope>();
        /*
         * link scope with parent_scope.
         * They must be linked in order to find_variables recursively.
         */
        auto parent_scope = m_scope_stack.top();
        if ( parent_scope )
        {
            m_graph->connect({EdgeType::IS_CHILD_OF, scope_node, parent_scope->get_owner()});
        }

        m_scope_stack.push( scope );

        scope->set_begin_scope_token(m_token_ribbon.getEaten());

        parse_code_block(false);

        if ( !m_token_ribbon.eatToken(Token_t::end_scope))
        {
            m_graph->destroy(scope_node);
            rollback_transaction();
            result = nullptr;
        }
        else
        {
            scope->set_end_scope_token(m_token_ribbon.getEaten());
            commit_transaction();
            result = scope_node;
        }

        m_scope_stack.pop();
    }
    return result;
}

IScope* Parser::parse_code_block(bool _create_scope)
{
    start_transaction();

    auto curr_scope = _create_scope ? m_graph->create_scope()->get<Scope>() : get_current_scope();

    NODABLE_ASSERT(curr_scope); // needed

    bool stop = false;

    while(m_token_ribbon.canEat() && !stop )
    {
        if ( InstructionNode* instr_node = parse_instr() )
        {
            m_graph->connect({EdgeType::IS_CHILD_OF, instr_node, m_scope_stack.top()->get_owner()});
        }
        else if ( parse_conditional_structure() ) {}
        else if ( parse_for_loop() ) {}
        else if ( parse_scope() ) {}
        else
        {
            stop = true;
        }
    }

    if (curr_scope->get_owner()->children_slots().empty() )
    {
        rollback_transaction();
        return nullptr;
    }
    else
    {
        commit_transaction();
        return curr_scope;
    }
}

Member* Parser::parse_expression(unsigned short _precedence, Member *_leftOverride)
{
	LOG_VERBOSE("Parser", "parse expr...\n")
	LOG_VERBOSE("Parser", "%s \n", m_token_ribbon.toString().c_str())

	if ( !m_token_ribbon.canEat() )
	{
		LOG_VERBOSE("Parser", "parse expr..." KO " (unable to eat a single token)\n")
        return _leftOverride;
	}

	/*
		Get the left handed operand
	*/
	Member* left = _leftOverride;
  if ( left == nullptr ) left = parse_parenthesis_expression();
  if ( left == nullptr ) left = parse_unary_operator_expression(_precedence);
  if ( left == nullptr ) left = parse_function_call();
  if ( left == nullptr ) left = parse_variable_declaration();
  if ( left == nullptr ) left = parse_atomic_expression();

	if ( !m_token_ribbon.canEat() )
	{
		LOG_VERBOSE("Parser", "parse expr... " OK " (last token reached)\n")
	}

	Member* result;

	/*
		Get the right handed operand
	*/
	if ( left )
	{
		LOG_VERBOSE("Parser", "parse expr... left parsed, we parse right\n")
		auto binResult = parse_binary_operator_expression(_precedence, left);

		if (binResult)
		{
			LOG_VERBOSE("Parser", "parse expr... right parsed, recursive call\n")
			result = parse_expression(_precedence, binResult);
		}
		else
        {
			result = left;
		}

	}
	else
    {
		LOG_VERBOSE("Parser", "parse expr... left is nullptr, we return it\n")
		result = left;
	}

	return result;
}

bool Parser::is_syntax_valid()
{
    // TODO: optimization: is this function really useful ? It check only few things.
    //                     The parsing steps that follow (parseProgram) is doing a better check, by looking to what exist in the Language.
	bool success   = true;
    auto currTokIt = m_token_ribbon.tokens.begin();
	short int opened = 0;

	while(currTokIt != m_token_ribbon.tokens.end() && success)
	{
		switch ( (*currTokIt)->m_type)
		{
            case Token_t::open_bracket:
            {
                opened++;
                break;
            }
            case Token_t::close_bracket:
            {
                opened--;

                if (opened < 0)
                {
                    LOG_VERBOSE("Parser", "Unexpected %s\n", (*currTokIt)->m_word.c_str())
                    success = false;
                }

                break;
            }
            default:
                break;
		}

		std::advance(currTokIt, 1);
	}

	if (opened != 0) // same opened/closed parenthesis count required.
    {
        LOG_VERBOSE("Parser", "bracket count mismatch, %i still opened.\n", opened)
        success = false;
    }

	return success;
}

bool Parser::tokenize_string(const std::string &_string)
{
    std::string::const_iterator cursor = _string.cbegin(); // the current char
    std::string                 pending_ignored_chars;

    /* shortcuts to language members */
    const Semantic& semantic = m_language.get_semantic();
    auto& regex              = semantic.get_token_type_regex();
    auto& regexIdToTokType   = semantic.get_token_type_regex_index_to_token_type();

    // Parsing method #1: loop over all regex (might be slow).
    auto parse_token_using_regexes = [&]() -> auto
    {
        int i = 0;
        for (auto&& eachRegexIt = regex.cbegin(); eachRegexIt != regex.cend(); eachRegexIt++)
        {
            i++;
            std::smatch sm;
            auto match = std::regex_search(cursor, _string.cend(), sm, *eachRegexIt);

            if (match)
            {
                std::string matched_str = sm.str();
                Token_t   matched_token_t   = regexIdToTokType[std::distance(regex.cbegin(), eachRegexIt)];

                if (matched_token_t != Token_t::ignore) {
                    size_t index = std::distance(_string.cbegin(), cursor);
                    auto new_token = std::make_shared<Token>(matched_token_t, matched_str, index);
                    LOG_VERBOSE("Parser", "tokenize <word>%s</word>\n", matched_str.c_str())

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
                                 last_token->m_suffix.append(pending_ignored_chars);
                                 pending_ignored_chars.clear();
                             }
                             else
                             {
                                 /*
                                 * Option 2: prefix of next/new token
                                 *
                                 * ( ... , <last_token>, <pending_ignored_chars><new-token>)
                                 */
                                 new_token->m_prefix.append(pending_ignored_chars);
                                 pending_ignored_chars.clear();
                             }
                         }
                        else
                        {
                            /*
                            * Option 3: prefix of the ribbon
                            *
                            * <pending_ignored_chars> (<first-and-new-token>)
                            */
                            m_token_ribbon.m_prefix->m_word = pending_ignored_chars;
                            pending_ignored_chars.clear();
                        }
                    }
                    m_token_ribbon.push( new_token );
                }
                else
                {
                    /*
                     * When a character is ignored, it goes to a pending_chars string.
                     * Once a no ignored token is parsed those pending_chars are added to the suffix (option 1) or to the prefix (option 2)
                     */
                    LOG_VERBOSE("Parser", "append ignored <word>%s</word>\n", matched_str.c_str() )
                    pending_ignored_chars.append(matched_str);
                }

                // advance iterator to the end of the str
                std::advance(cursor, matched_str.length());
                return true;
            }
        }
        return false;
    };

    // Parsing method #2: should be faster (that's the objective)
    auto parse_token_fast = [&]() -> auto
    {
        return false;
    };

	while( cursor != _string.cend())
	{
	    // first, we try to tokenize using a WIP technique not involving any regex
	    if(parse_token_fast())
	    {
	        continue;
	    }

	    // then, if nothing matches, we try using our old technique using regexes
		if (parse_token_using_regexes()) continue;

		size_t distance = std::distance(_string.cbegin(), cursor);
		LOG_WARNING("Parser", "tokenizing failed! Unable to tokenize at index %llu\n", distance )
		return false;
	}

	/*
	 * Append pending ignored chars
	 */
	if ( !pending_ignored_chars.empty() )
    {
        m_token_ribbon.m_suffix->m_word = pending_ignored_chars;
        pending_ignored_chars.clear();
    }
	return true;
}

Member* Parser::parse_function_call()
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
        token_1->m_type == Token_t::open_bracket)
    {
        fct_id = token_0->m_word;
        LOG_VERBOSE("Parser", "parse function call... " OK " regular function pattern detected.\n")
    }
    else // Try to parse operator like (ex: operator==(..,..))
    {
        std::shared_ptr<Token> token_2 = m_token_ribbon.eatToken(); // eat a "supposed open bracket>

        if (   token_0->m_type == Token_t::identifier
            && token_0->m_word == m_language.get_semantic().token_type_to_string(Token_t::keyword_operator )
            && token_1->m_type == Token_t::operator_
            && token_2->m_type == Token_t::open_bracket)
        {
            // ex: "operator" + "=="
            fct_id = token_0->m_word + token_1->m_word;
            LOG_VERBOSE("Parser", "parse function call... " OK " operator function-like pattern detected.\n")
        }
        else
        {
            LOG_VERBOSE("Parser", "parse function call... " KO " abort, this is not a function.\n")
            rollback_transaction();
            return nullptr;
        }
    }
    std::vector<Member *> args;

    // Declare a new function prototype
    Signature signature(fct_id);
    signature.set_return_type(type::any);

    bool parsingError = false;
    while (!parsingError && m_token_ribbon.canEat() && m_token_ribbon.peekToken()->m_type != Token_t::close_bracket)
    {

        if (auto member = parse_expression())
        {
            args.push_back(member); // store argument as member (already parsed)
            signature.push_arg(member->get_type());  // add a new argument type to the proto.
            m_token_ribbon.eatToken(Token_t::separator);
        }
        else
        {
            parsingError = true;
        }
    }

    // eat "close bracket supposed" token
    if ( !m_token_ribbon.eatToken(Token_t::close_bracket) )
    {
        LOG_WARNING("Parser", "parse function call... " KO " abort, close parenthesis expected. \n")
        rollback_transaction();
        return nullptr;
    }


    // Find the prototype in the language library
    const IInvokable* invokable = m_language.find_function(&signature);

    auto connectArg = [&](const Signature* _sig, Node* _node, size_t _arg_index ) -> void
    { // lambda to connect input member to node for a specific argument index.
        Member*     src_member      = args.at(_arg_index);
        Member*     dst_member      = _node->props()->get_input_at(_arg_index);
        NODABLE_ASSERT(dst_member)
        m_graph->connect(src_member, dst_member);
    };

    Node* node;
    if (invokable)
    {
        /*
         * If we found a function matching signature, we create a node with that function.
         * The node will be able to be evaluated.
         *
         * TODO: remove this method, the parser should not check if function exist or not.
         *       this role is for the Compiler.
         */
        node = m_graph->create_function(invokable);
    }
    else
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

    return node->props()->get(k_value_member_name);
}

Scope* Parser::get_current_scope()
{
    NODABLE_ASSERT(m_scope_stack.top()); // stack SHALL not be empty.
    return m_scope_stack.top();
}

ConditionalStructNode * Parser::parse_conditional_structure()
{
    LOG_VERBOSE("Parser", "try to parse conditional structure...\n")
    start_transaction();

    bool success = false;
    ConditionalStructNode* condStruct = m_graph->create_cond_struct();

    if ( m_token_ribbon.eatToken(Token_t::keyword_if))
    {
        m_graph->connect({condStruct, EdgeType::IS_CHILD_OF, m_scope_stack.top()->get_owner()});
        m_scope_stack.push( condStruct->get<Scope>() );

        condStruct->set_token_if(m_token_ribbon.getEaten());

        if(m_token_ribbon.eatToken(Token_t::open_bracket))
        {
            InstructionNode* condition = parse_instr();

            if ( condition)
            {
                condition->set_label("Condition");
                condition->set_label("Cond.");
                condStruct->set_cond_instr(condition);

                if ( m_token_ribbon.eatToken(Token_t::close_bracket) )
                {
                    m_graph->connect(condition->get_this_member(), condStruct->condition_member() );

                    if ( Node* scopeIf = parse_scope() )
                    {
                        if ( m_token_ribbon.eatToken(Token_t::keyword_else))
                        {
                            condStruct->set_token_else(m_token_ribbon.getEaten());

                            /* parse else scope */
                            if ( parse_scope() )
                            {
                                LOG_VERBOSE("Parser", "parse IF {...} ELSE {...} block... " OK "\n")
                                success = true;
                            }
                            /* (or) parse else if scope */
                            else if ( ConditionalStructNode* else_cond_struct = parse_conditional_structure() )
                            {
                                else_cond_struct->set_label("else if");

                                LOG_VERBOSE("Parser", "parse IF {...} ELSE IF {...} block... " OK "\n")
                                success = true;
                            }
                            else
                            {
                                LOG_VERBOSE("Parser", "parse IF {...} ELSE {...} block... " KO "\n")
                                m_graph->destroy(scopeIf);
                            }
                        }
                        else
                        {
                             LOG_VERBOSE("Parser", "parse IF {...} block... " OK "\n")
                            success = true;
                        }
                    }
                    else
                    {
                        if ( condition )
                        {
                            m_graph->destroy(condition);
                        }
                        LOG_VERBOSE("Parser", "parse IF {...} block... " KO "\n")
                    }
                }
                else
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
    }
    else
    {
        m_graph->destroy(condStruct);
        condStruct = nullptr;
        rollback_transaction();
    }

    return condStruct;
}

ForLoopNode* Parser::parse_for_loop()
{
    bool success = false;
    ForLoopNode* for_loop_node = nullptr;
    start_transaction();

    std::shared_ptr<Token> token_for = m_token_ribbon.eatToken(Token_t::keyword_for);

    if( token_for != nullptr )
    {
        for_loop_node = m_graph->create_for_loop();
        m_graph->connect({for_loop_node, EdgeType::IS_CHILD_OF, m_scope_stack.top()->get_owner() });
        m_scope_stack.push( for_loop_node->get<Scope>() );

        for_loop_node->set_token_for( token_for );

        LOG_VERBOSE("Parser", "parse FOR (...) block...\n")
        std::shared_ptr<Token> open_bracket = m_token_ribbon.eatToken(Token_t::open_bracket);
        if( !open_bracket )
        {
            LOG_ERROR("Parser", "Unable to find open bracket after for keyword.\n")
        }
        else
        {
            InstructionNode* init_instr = parse_instr();
            if (!init_instr)
            {
                LOG_ERROR("Parser", "Unable to find initial instruction.\n")
            }
            else
            {
                init_instr->set_label("Initialisation", "Init.");
                m_graph->connect(init_instr->get_this_member(), for_loop_node->get_init_expr());
                for_loop_node->set_init_instr(init_instr);

                InstructionNode* cond_instr = parse_instr();
                if (!cond_instr)
                {
                    LOG_ERROR("Parser", "Unable to find condition instruction.\n")
                }
                else
                {
                    cond_instr->set_label("Condition", "Cond.");
                    m_graph->connect(cond_instr->get_this_member(), for_loop_node->condition_member());
                    for_loop_node->set_cond_instr(cond_instr);

                    InstructionNode* iter_instr = parse_instr();
                    if (!iter_instr)
                    {
                        LOG_ERROR("Parser", "Unable to find iterative instruction.\n")
                    }
                    else
                    {
                        iter_instr->set_label("Iteration", "Iter.");
                        m_graph->connect(iter_instr->get_this_member(), for_loop_node->get_iter_expr());
                        for_loop_node->set_iter_instr(iter_instr);

                        std::shared_ptr<Token> close_bracket = m_token_ribbon.eatToken(Token_t::close_bracket);
                        if (!close_bracket)
                        {
                            LOG_ERROR("Parser", "Unable to find close bracket after iterative instruction.\n")
                        }
                        else if (!parse_scope())
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
        m_scope_stack.pop();
    }

    if ( success )
    {
        commit_transaction();
    }
    else
    {
        rollback_transaction();
        if ( for_loop_node ) m_graph->destroy(for_loop_node);
        for_loop_node = nullptr;
    }

    return for_loop_node;
}

Member *Parser::parse_variable_declaration()
{

    if( !m_token_ribbon.canEat(2))
        return nullptr;

    start_transaction();

    std::shared_ptr<Token> tok_type       = m_token_ribbon.eatToken();
    std::shared_ptr<Token> tok_identifier = m_token_ribbon.eatToken();

    if(tok_type->is_keyword_type() && tok_identifier->m_type == Token_t::identifier )
    {
        type type = m_language.get_semantic().token_type_to_type(tok_type->m_type);
        VariableNode* variable = m_graph->create_variable(type, tok_identifier->m_word, get_current_scope());
        variable->set_type_token(tok_type);
        variable->set_identifier_token(tok_identifier);
        variable->get_value()->set_src_token( std::make_shared<Token>(*tok_identifier) );

        // try to parse assignment
        std::shared_ptr<Token> assignmentTok = m_token_ribbon.eatToken(Token_t::operator_);
        if ( assignmentTok && assignmentTok->m_word == "=" )
        {
            auto expression_result = parse_expression();
            if( expression_result &&
                    type::is_implicitly_convertible(expression_result->get_type()
                                                           , variable->get_value()->get_type()) )
            {
                m_graph->connect( expression_result, variable );
                variable->set_assignment_operator_token(assignmentTok);
            }
            else
            {
                LOG_ERROR("Parser", "Unable to parse expression to assign %s\n", tok_identifier->m_word.c_str())
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
