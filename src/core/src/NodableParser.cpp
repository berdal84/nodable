#include <nodable/core/languages/NodableParser.h>

#include <algorithm>
#include <memory> // std::shared_ptr
#include <regex>
#include <sstream>
#include <string>

#include <nodable/core/ConditionalStructNode.h>
#include <nodable/core/GraphNode.h>
#include <nodable/core/InstructionNode.h>
#include <nodable/core/InvokableComponent.h>
#include <nodable/core/LiteralNode.h>
#include <nodable/core/Log.h>
#include <nodable/core/Member.h>
#include <nodable/core/Operator.h>
#include <nodable/core/Scope.h>
#include <nodable/core/reflection/func_type.h>
#include <nodable/core/System.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/Wire.h>
#include <nodable/core/languages/NodableLanguage.h>

using namespace ndbl;

NodableParser::NodableParser(const NodableLanguage& _lang, bool _strict )
        : m_language(_lang)
        , m_strict_mode(_strict)
        , m_graph(nullptr){}

void NodableParser::rollback_transaction()
{
    m_token_ribbon.rollbackTransaction();
}

void NodableParser::start_transaction()
{
    m_token_ribbon.startTransaction();
}

void NodableParser::commit_transaction()
{
    m_token_ribbon.commitTransaction();
}

bool NodableParser::parse(const std::string &_source_code, GraphNode *_graphNode)
{
    m_graph = _graphNode;
    m_token_ribbon.clear();

    LOG_VERBOSE("Parser", "Trying to evaluate evaluated: <expr>%s</expr>\"\n", _source_code.c_str() )
    LOG_MESSAGE("Parser", "Tokenization ...\n" )


    if (!tokenize(_source_code))
    {
        return false;
    }

	if (!is_syntax_valid())
	{
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
        auto curr_token = m_token_ribbon.peekToken();
        LOG_WARNING("Parser", "Unable to handle the token %s (char: %llu).\n"
                     , curr_token->m_word.c_str()
                     , curr_token->m_charIndex )
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

bool NodableParser::to_bool(const std::string &_str)
{
    return _str == std::string("true");
}

std::string NodableParser::to_string(const std::string& _quoted_str)
{
    NDBL_ASSERT(_quoted_str.size() >= 2);
    NDBL_ASSERT(_quoted_str.front() == '\"');
    NDBL_ASSERT(_quoted_str.back() == '\"');
    return std::string(++_quoted_str.cbegin(), --_quoted_str.cend());
}

double NodableParser::to_double(const std::string &_str)
{
    return stod(_str);
}

i16_t NodableParser::to_i16(const std::string &_str)
{
    return stoi(_str);
}

Member* NodableParser::to_member(std::shared_ptr<Token> _token)
{
    if( _token->m_type == Token_t::identifier )
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
        if (variable)
        {
            return variable->get_value();
        }
        return nullptr;
    }

    LiteralNode* literal = nullptr;

	switch (_token->m_type)
	{
	    case Token_t::literal_bool:
        {
            literal = m_graph->create_literal(type::get<bool>());
            literal->set_value(to_bool(_token->m_word) );
            break;
        }

	    case Token_t::literal_int:
        {
            literal = m_graph->create_literal(type::get<i16_t>());
            literal->set_value(to_i16(_token->m_word) );
            break;
        }

	    case Token_t::literal_double:
        {
            literal = m_graph->create_literal(type::get<double>());
            literal->set_value(to_double(_token->m_word) );
            break;
        }

	    case Token_t::literal_string:
        {
            literal = m_graph->create_literal(type::get<std::string>());
            literal->set_value(to_string(_token->m_word) );
            break;
        }

        default: ;
    }

    if ( literal )
    {
        Member* result = literal->get_value();
        result->set_src_token(_token);
        return result;
    }

    LOG_VERBOSE("Parser", "Unable to perform token_to_member for token %s!\n", _token->m_word.c_str())
	return nullptr;
}

Member* NodableParser::parse_binary_operator_expression(unsigned short _precedence, Member *_left) {

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
	func_type* type = new func_type(ope->identifier);
    type->set_return_type(type::any);
    type->push_args(_left->get_type(), right->get_type());

    InvokableComponent* component;
    Node*               binary_op;

    if ( auto invokable = m_language.find_operator_fct(type) )
	{
	    // concrete operator
		binary_op = m_graph->create_operator(invokable.get());
        component = binary_op->get<InvokableComponent>();
        delete type;
    }
	else if(type)
    {
	    // abstract operator
        binary_op = m_graph->create_abstract_operator(type);
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

Member* NodableParser::parse_unary_operator_expression(unsigned short _precedence)
{
	LOG_VERBOSE("Parser", "parseUnaryOperationExpression...\n")
	LOG_VERBOSE("Parser", "%s \n", m_token_ribbon.toString().c_str())

	if (!m_token_ribbon.canEat(2) )
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
    func_type* type = new func_type(operator_token->m_word);
    type->set_return_type(type::any);
    type->push_args(value->get_type());

    InvokableComponent* component;
	Node*               node;

	if (auto invokable = m_language.find_operator_fct(type))
	{
        node = m_graph->create_operator(invokable.get());
        delete type;
	}
	else if(type)
	{
        node = m_graph->create_abstract_operator(type);
	}
    else
    {
        LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (no signature found)\n")
        rollback_transaction();
        return nullptr;
    }

    component = node->get<InvokableComponent>();
    component->set_source_token(operator_token);

    m_graph->connect(value, component->get_l_handed_val());
    Member* result = node->props()->get(k_value_member_name);

    LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " OK "\n")
    commit_transaction();

    return result;
}

Member* NodableParser::parse_atomic_expression()
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

	auto result = to_member(token);

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

Member* NodableParser::parse_parenthesis_expression()
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
	if (currentToken->m_type != Token_t::fct_params_begin)
	{
		LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " open bracket not found.\n")
        rollback_transaction();
		return nullptr;
	}

    Member* result = parse_expression();
	if (result)
	{
        std::shared_ptr<Token> token = m_token_ribbon.eatToken();
		if (token->m_type != Token_t::fct_params_end )
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

InstructionNode* NodableParser::parse_instr()
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
        else if( m_token_ribbon.peekToken()->m_type != Token_t::fct_params_end )
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

Node* NodableParser::parse_program()
{
    start_transaction();

    m_graph->clear();
    Node*  root          = m_graph->create_root();
    Scope* program_scope = root->get<Scope>();
    m_scope_stack.push( program_scope );

    parse_code_block(false); // we do not check if we parsed something empty or not, a program can be empty.

    // Add ignored chars pre/post token to the main scope begin/end token prefix/suffix.
    NDBL_ASSERT(!program_scope->get_begin_scope_token())
    NDBL_ASSERT(!program_scope->get_end_scope_token())
    program_scope->set_begin_scope_token( m_token_ribbon.m_prefix );
    program_scope->set_end_scope_token( m_token_ribbon.m_suffix );

    m_scope_stack.pop();
    commit_transaction();

    return m_graph->get_root();
}

Node* NodableParser::parse_scope()
{
    Node* result;

    start_transaction();

    if ( !m_token_ribbon.eatToken(Token_t::scope_begin))
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

        if ( !m_token_ribbon.eatToken(Token_t::scope_end))
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

IScope* NodableParser::parse_code_block(bool _create_scope)
{
    start_transaction();

    auto curr_scope = _create_scope ? m_graph->create_scope()->get<Scope>() : get_current_scope();

    NDBL_ASSERT(curr_scope); // needed

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

Member* NodableParser::parse_expression(unsigned short _precedence, Member *_leftOverride)
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

bool NodableParser::is_syntax_valid()
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
            case Token_t::fct_params_begin:
            {
                opened++;
                break;
            }
            case Token_t::fct_params_end:
            {
                if ( opened <= 0)
                {
                    LOG_ERROR("Parser", "Syntax Error: Unexpected close bracket after \"... %s\" (position %llu)\n"
                              , m_token_ribbon.get_words( (*currTokIt)->m_index, -10 ).c_str()
                              , (*currTokIt)->m_charIndex )
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

	if (opened > 0) // same opened/closed parenthesis count required.
    {
        LOG_ERROR("Parser", "Syntax Error: Bracket count mismatch, %i still opened.\n", opened)
        success = false;
    }

	return success;
}

bool NodableParser::tokenize(const std::string& _string)
{
    std::string::const_iterator cursor = _string.cbegin(); // the current char
    std::string                 pending_ignored_chars;

    /* shortcuts to language members */
    auto& regex              = m_language.get_token_type_regex();
    auto& regexIdToTokType   = m_language.get_token_type_regex_index_to_token_type();

    // Parsing method #1: loop over all regex (might be slow).
    auto parse_token_using_regexes = [&]() -> std::shared_ptr<Token>
    {
        std::shared_ptr<Token> result;
        size_t index = std::distance(_string.cbegin(), cursor);

        for (auto&& each_regex_it = regex.cbegin(); each_regex_it != regex.cend(); each_regex_it++)
        {
            std::smatch sm;
            auto match = std::regex_search(cursor, _string.cend(), sm, *each_regex_it);

            if (match)
            {
                std::string matched_str = sm.str();
                Token_t matched_token_t = regexIdToTokType[std::distance(regex.cbegin(), each_regex_it)];
                result                  = std::make_shared<Token>(matched_token_t, matched_str, index);
                std::advance(cursor, matched_str.length());

                LOG_VERBOSE("Parser", "Token identified as %s at char %i: %s\n", m_language.to_string(matched_token_t).c_str(),  index, matched_str.c_str())

                return result;
            }
        }
        return result;
    };

    // Parsing method #2: should be faster (that's the objective)
    auto parse_token_fast = [&]() -> std::shared_ptr<Token>
    {
        std::shared_ptr<Token>  result;
        size_t                  cursor_idx   = std::distance(_string.cbegin(), cursor );
        size_t                  char_left    = _string.size() - cursor_idx;
        std::string::value_type current_char = _string.data()[cursor_idx];

        // single char
        switch ( current_char )
        {
            case '{': cursor++; return std::make_shared<Token>(Token_t::scope_begin          , current_char, cursor_idx);
            case '}': cursor++; return std::make_shared<Token>(Token_t::scope_end            , current_char, cursor_idx);
            case '(': cursor++; return std::make_shared<Token>(Token_t::fct_params_begin     , current_char, cursor_idx);
            case ')': cursor++; return std::make_shared<Token>(Token_t::fct_params_end       , current_char, cursor_idx);
            case ',': cursor++; return std::make_shared<Token>(Token_t::fct_params_separator , current_char, cursor_idx);
            case ';': cursor++; return std::make_shared<Token>(Token_t::end_of_instruction   , current_char, cursor_idx);
            case '\n': // fallthrough ...
            case ' ':  // fallthrough ...
            case '\t': cursor++; return std::make_shared<Token>(Token_t::ignore , current_char, cursor_idx);
            default: /* no warning */ ;
        }

        // booleans
        if( _string.compare(cursor_idx, 4, "true") == 0)
        {
            result = std::make_shared<Token>(Token_t::literal_bool, "true", cursor_idx);
            std::advance(cursor, 4);
            return result;
        }
        if( _string.compare(cursor_idx, 5, "false") == 0 )
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

	while( cursor != _string.cend())
	{
	    std::shared_ptr<Token> new_token;

	    // first, we try to tokenize using a WIP technique not involving any regex
	    new_token = parse_token_fast();

	    // then, if nothing matches, we try using our old technique using regexes
		if( !new_token )
        {
            new_token = parse_token_using_regexes();
        }

		if ( !new_token )
        {
            size_t distance = std::distance(_string.cbegin(), cursor);
            LOG_WARNING("Parser", "Scanner Error: unable to tokenize %s at index %llu\n", _string.substr(distance, 10).c_str(),  distance )
            return false;
        }

        // Finally we push the token in the ribbon
        if(new_token->m_type != Token_t::ignore)
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
            m_token_ribbon.push(new_token );
            LOG_VERBOSE("Parser", "Append: %s\n", Token::to_string(new_token).c_str() )
        }
        else
        {
            /*
             * When a character is ignored, it goes to a pending_chars string.
             * Once a no ignored token is parsed those pending_chars are added to the suffix (option 1) or to the prefix (option 2)
             */
            pending_ignored_chars.append(new_token->m_word);
            LOG_VERBOSE("Parser", "Append ignored: %s\n", Token::to_string(new_token).c_str() )
        }
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

Member* NodableParser::parse_function_call()
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
        fct_id = token_0->m_word;
        LOG_VERBOSE("Parser", "parse function call... " OK " regular function pattern detected.\n")
    }
    else // Try to parse operator like (ex: operator==(..,..))
    {
        std::shared_ptr<Token> token_2 = m_token_ribbon.eatToken(); // eat a "supposed open bracket>

        if (   token_0->m_type == Token_t::keyword_operator
            && token_1->m_type == Token_t::operator_
            && token_2->m_type == Token_t::fct_params_begin)
        {
            fct_id = token_1->m_word; // operator
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
    func_type signature(fct_id);
    signature.set_return_type(type::any);

    bool parsingError = false;
    while (!parsingError && m_token_ribbon.canEat() && m_token_ribbon.peekToken()->m_type != Token_t::fct_params_end)
    {

        if (auto member = parse_expression())
        {
            args.push_back(member); // store argument as member (already parsed)
            signature.push_arg(member->get_type());  // add a new argument type to the proto.
            m_token_ribbon.eatToken(Token_t::fct_params_separator);
        }
        else
        {
            parsingError = true;
        }
    }

    // eat "close bracket supposed" token
    if ( !m_token_ribbon.eatToken(Token_t::fct_params_end) )
    {
        LOG_WARNING("Parser", "parse function call... " KO " abort, close parenthesis expected. \n")
        rollback_transaction();
        return nullptr;
    }


    // Find the prototype in the language library
    std::shared_ptr<const iinvokable> invokable = m_language.find_function(&signature);

    auto connectArg = [&](const func_type* _sig, Node* _node, size_t _arg_index ) -> void
    { // lambda to connect input member to node for a specific argument index.
        Member*     src_member      = args.at(_arg_index);
        Member*     dst_member      = _node->props()->get_input_at(_arg_index);
        NDBL_ASSERT(dst_member)
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
        node = m_graph->create_function(invokable.get());
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

Scope* NodableParser::get_current_scope()
{
    NDBL_ASSERT(m_scope_stack.top()); // stack SHALL not be empty.
    return m_scope_stack.top();
}

ConditionalStructNode * NodableParser::parse_conditional_structure()
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

        if(m_token_ribbon.eatToken(Token_t::fct_params_begin))
        {
            InstructionNode* condition = parse_instr();

            if ( condition)
            {
                condition->set_label("Condition");
                condition->set_label("Cond.");
                condStruct->set_cond_instr(condition);

                if ( m_token_ribbon.eatToken(Token_t::fct_params_end) )
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

ForLoopNode* NodableParser::parse_for_loop()
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
        std::shared_ptr<Token> open_bracket = m_token_ribbon.eatToken(Token_t::fct_params_begin);
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

                        std::shared_ptr<Token> close_bracket = m_token_ribbon.eatToken(Token_t::fct_params_end);
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

Member *NodableParser::parse_variable_declaration()
{

    if( !m_token_ribbon.canEat(2))
        return nullptr;

    start_transaction();

    std::shared_ptr<Token> tok_type       = m_token_ribbon.eatToken();
    std::shared_ptr<Token> tok_identifier = m_token_ribbon.eatToken();

    if(tok_type->is_keyword_type() && tok_identifier->m_type == Token_t::identifier )
    {
        type type = m_language.get_type(tok_type->m_type);
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
