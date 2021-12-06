#include <nodable/Parser.h>

#include <regex>
#include <algorithm>
#include <sstream>
#include <string>

#include <nodable/Log.h>
#include <nodable/Member.h>
#include <nodable/Wire.h>
#include <nodable/GraphNode.h>
#include <nodable/InstructionNode.h>
#include <nodable/CodeBlockNode.h>
#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/LiteralNode.h>
#include <nodable/VariableNode.h>
#include <nodable/InvokableComponent.h>

using namespace Nodable;

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

bool Parser::expression_to_graph(const std::string &_code,
                                 GraphNode *_graphNode)
{
    m_graph = _graphNode;
    m_token_ribbon.clear();

    LOG_VERBOSE("Parser", "Trying to evaluate evaluated: <expr>%s</expr>\"\n", _code.c_str() )

    std::istringstream iss(_code);
    std::string line;
    std::string eol;
    m_language->getSerializer()->serialize(eol, TokenType_EndOfLine);

    size_t lineCount = 0;
    while (std::getline(iss, line, eol[0] ))
    {
        if ( lineCount != 0 && !m_token_ribbon.tokens.empty() )
        {
            Token* lastToken = &m_token_ribbon.tokens.back();
            lastToken->m_suffix.append(eol);
        }

        if (!tokenize_string(line))
        {
            LOG_WARNING("Parser", "Unable to parse code due to unrecognized tokens.\n")
            return false;
        }

        lineCount++;
    }

	if (m_token_ribbon.empty() )
    {
        LOG_MESSAGE("Parser", "Empty code. Nothing to evaluate.\n")
        return false;
    }

	if (!is_syntax_valid())
	{
		LOG_WARNING("Parser", "Unable to parse code due to syntax error.\n")
		return false;
	}

	CodeBlockNode* program = parse_program();

	if (program == nullptr)
	{
		LOG_WARNING("Parser", "Unable to parse main scope due to abstract syntax tree failure.\n")
		return false;
	}

    if ( m_token_ribbon.canEat() )
    {
        m_graph->clear();
        LOG_ERROR("Parser", "Unable to evaluate the full expression.\n")
        return false;
    }

    // We unset dirty, since we did a lot of connections but we don't want any update now
    auto& nodes = m_graph->getNodeRegistry();
    for(auto eachNode : nodes )
        eachNode->setDirty(false);

	LOG_MESSAGE("Parser", "Graph well updated.\n", _code.c_str() )
	LOG_VERBOSE("Parser", "Expression evaluated: <expr>%s</expr>\"\n", _code.c_str() )
	return true;
}

Type Parser::get_literal_type(const Nodable::Token *_token) const
{
    Type type = Type_Unknown;

    const std::vector<std::regex> regex            = m_language->getSemantic()->get_type_regex();
    const std::vector<Type>       regex_id_to_type = m_language->getSemantic()->get_type_regex_index_to_type();

    auto each_regex_it = regex.cbegin();
    while( each_regex_it != regex.cend() && type == Type_Unknown )
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

    NODABLE_ASSERT(type != Type_Unknown)

    return type;
}

bool Parser::parse_bool(const std::string &_str)
{
    return _str == std::string("true");
}

std::string Parser::parse_string(const std::string &_str)
{
    NODABLE_ASSERT(_str.size() >= 2);
    NODABLE_ASSERT(_str[0] == '\"');
    NODABLE_ASSERT(_str[_str.size()-1] == '\"');
    return std::string(++_str.cbegin(), --_str.cend());
}

double Parser::parse_double(const std::string &_str)
{
    return stod(_str);
}

Member* Parser::token_to_member(Token *_token)
{
	Member* result = nullptr;

	switch (_token->m_type)
	{

	    case TokenType_Literal:
        {
            Type type = get_literal_type(_token);
            LiteralNode* literal = m_graph->newLiteral(type);

            switch ( type ) {
                case Type_String: literal->set_value(parse_string(_token->m_word) ); break;
                case Type_Double: literal->set_value(parse_double(_token->m_word) ); break;
                case Type_Boolean: literal->set_value(parse_bool(_token->m_word)  ); break;
                default: {}
            }

            result = literal->get_value();
            result->setSourceToken(_token);
            break;
        }

		case TokenType_Identifier:
		{
			VariableNode* variable = m_graph->findVariable(_token->m_word);

			if (variable == nullptr) {
                LOG_WARNING("Parser", "Unable to find declaration for %s \n", _token->m_word.c_str())
                variable = m_graph->newVariable(Type_Any, _token->m_word, get_current_scope() );
                variable->value()->setSourceToken(_token);
            }

            result = variable->value();
			break;
		}

	    default:
	        assert("This TokenType is not handled by this method.");

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
	Token* operatorToken = m_token_ribbon.eatToken();
	const Token* token2 = m_token_ribbon.peekToken();

	// Structure check
	const bool isValid = _left != nullptr &&
                         operatorToken->m_type == TokenType_Operator &&
                         token2->m_type != TokenType_Operator;

	if (!isValid)
	{
        rollback_transaction();
		LOG_VERBOSE("Parser", "parse binary operation expr... " KO " (Structure)\n")
		return nullptr;
	}

	// Precedence check
	const auto currentOperatorPrecedence = m_language->findOperator(operatorToken->m_word)->get_precedence();

	if (currentOperatorPrecedence <= _precedence &&
	    _precedence > 0u) { // always update the first operation if they have the same precedence or less.
		LOG_VERBOSE("Parser", "parse binary operation expr... " KO " (Precedence)\n")
        rollback_transaction();
		return nullptr;
	}


	// Parse right expression
	auto right = parse_expression(currentOperatorPrecedence, nullptr);

	if (!right)
	{
		LOG_VERBOSE("Parser", "parseBinaryOperationExpression... " KO " (right expression is nullptr)\n")
        rollback_transaction();
		return nullptr;
	}

	// Create a function signature according to ltype, rtype and operator word
	const FunctionSignature* signature = m_language->createBinOperatorSignature(Type_Any, operatorToken->m_word, _left->getType(), right->getType());
	auto matchingOperator = m_language->findOperator(signature);
    delete signature;

	if ( matchingOperator != nullptr )
	{
		auto binOpNode = m_graph->newBinOp(matchingOperator);
        auto computeComponent = binOpNode->getComponent<InvokableComponent>();
        computeComponent->set_source_token(operatorToken);

        m_graph->connect(_left, computeComponent->get_l_handed_val());
        m_graph->connect(right, computeComponent->get_r_handed_val());
		result = binOpNode->getProps()->get("result");

        commit_transaction();
        LOG_VERBOSE("Parser", "parse binary operation expr... " OK "\n")

        return result;
    }
    else
    {
        LOG_VERBOSE("Parser", "parse binary operation expr... " KO " (unable to find operator prototype)\n")
        rollback_transaction();
        return nullptr;
    }
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
	Token* operatorToken = m_token_ribbon.eatToken();

	// Check if we get an operator first
	if (operatorToken->m_type != TokenType_Operator)
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
	auto signature = m_language->createUnaryOperatorSignature(Type_Any, operatorToken->m_word, value->getType() );
	auto matchingOperator = m_language->findOperator(signature);

	if (matchingOperator != nullptr)
	{
		auto unaryOpNode = m_graph->newUnaryOp(matchingOperator);
        auto computeComponent = unaryOpNode->getComponent<InvokableComponent>();
        computeComponent->set_source_token(operatorToken);

        m_graph->connect(value, computeComponent->get_l_handed_val());
        Member* result = unaryOpNode->getProps()->get("result");

		LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " OK "\n")
        commit_transaction();

		return result;
	}
	else
	{
		LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (unrecognysed operator)\n")
        rollback_transaction();
		return nullptr;
	}
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
	Token* token = m_token_ribbon.eatToken();
	if (token->m_type == TokenType_Operator)
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
	const Token* currentToken = m_token_ribbon.eatToken();
	if (currentToken->m_type != TokenType_OpenBracket)
	{
		LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " open bracket not found.\n")
        rollback_transaction();
		return nullptr;
	}

    Member* result = parse_expression();
	if (result)
	{
        const Token* token = m_token_ribbon.eatToken();
		if (token->m_type != TokenType_CloseBracket )
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

InstructionNode* Parser::parse_instruction()
{
    start_transaction();

    Member* parsedExpression = parse_expression();

    if ( parsedExpression == nullptr )
    {
       LOG_VERBOSE("Parser", "parse instruction " KO " (parsed is nullptr)\n")
        rollback_transaction();
       return nullptr;
    }

    auto instruction = m_graph->newInstruction();

    if ( m_token_ribbon.canEat() )
    {
        Token* expectedEOI = m_token_ribbon.eatToken(TokenType_EndOfInstruction);
        if ( expectedEOI )
        {
            instruction->setEndOfInstrToken( expectedEOI );
        }
        else if( m_token_ribbon.peekToken()->m_type != TokenType_CloseBracket )
        {
            LOG_VERBOSE("Parser", "parse instruction " KO " (end of instruction not found)\n")
            rollback_transaction();
            return nullptr;
        }
    }

    m_graph->connect(parsedExpression, instruction);

    LOG_VERBOSE("Parser", "parse instruction " OK "\n")
    commit_transaction();
    return instruction;
}

CodeBlockNode* Parser::parse_program()
{
    start_transaction();
    ScopedCodeBlockNode* scope = m_graph->newProgram();
    if(CodeBlockNode* block = parse_code_block())
    {
        m_graph->connect(block, scope, RelationType::IS_CHILD_OF);
        commit_transaction();
        return block;
    }
    else
    {
        m_graph->clear();
        rollback_transaction();
        return nullptr;
    }
}

ScopedCodeBlockNode* Parser::parse_scope()
{
    start_transaction();

    if ( !m_token_ribbon.eatToken(TokenType_BeginScope))
    {
        rollback_transaction();
        return nullptr;
    }

    auto scope = m_graph->newScopedCodeBlock();
    scope->set_begin_scope_token(m_token_ribbon.getEaten());

    if ( auto block = parse_code_block() )
    {
        m_graph->connect(block, scope, RelationType::IS_CHILD_OF);
    }

    if ( !m_token_ribbon.eatToken(TokenType_EndScope))
    {
        m_graph->deleteNode(scope);
        rollback_transaction();
        return nullptr;
    }
    scope->set_end_Scope_token(m_token_ribbon.getEaten());

    commit_transaction();
    return scope;
}

CodeBlockNode* Parser::parse_code_block()
{
    start_transaction();

    auto block = m_graph->newCodeBlock();

    bool stop = false;

//    Node* previous = block;
    while(m_token_ribbon.canEat() && !stop )
    {
        if ( InstructionNode* instruction = parse_instruction() )
        {
            m_graph->connect(instruction, block, RelationType::IS_CHILD_OF);
        }
        else if ( ScopedCodeBlockNode* scope = parse_scope() )
        {
            m_graph->connect(scope, block, RelationType::IS_CHILD_OF);
        }
        else if ( ConditionalStructNode* condStruct = parse_conditional_structure() )
        {
            m_graph->connect(condStruct, block, RelationType::IS_CHILD_OF);
        }
        else if ( ForLoopNode* for_loop = parse_for_loop() )
        {
            m_graph->connect(for_loop, block, RelationType::IS_CHILD_OF);
        }
        else
        {
            stop = true;
        }
    }

    if (block->get_children().empty() )
    {
        m_graph->deleteNode(block);
        rollback_transaction();
        return nullptr;
    }
    else
    {
        commit_transaction();
        return block;
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
	short int openedParenthesisCount = 0;

	while(currTokIt != m_token_ribbon.tokens.end() && success)
	{
		switch (currTokIt->m_type)
		{
            case TokenType_OpenBracket:
            {
                openedParenthesisCount++;
                break;
            }
            case TokenType_CloseBracket:
            {
                openedParenthesisCount--;

                if (openedParenthesisCount < 0)
                {
                    LOG_VERBOSE("Parser", "Unexpected %s\n", currTokIt->m_word.c_str())
                    success = false;
                }

                break;
            }
            default:
                break;
		}

		std::advance(currTokIt, 1);
	}

	if (openedParenthesisCount != 0) // same opened/closed parenthesis count required.
    {
        LOG_VERBOSE("Parser", "bracket count mismatch, %i still opened.\n", openedParenthesisCount)
        success = false;
    }

	return success;
}

bool Parser::tokenize_string(const std::string &_expression)
{
    /* get expression chars */
    auto chars = _expression;

    /* shortcuts to language members */
    const std::vector<std::regex> regex           = m_language->getSemantic()->get_token_type_regex();
    const std::vector<TokenType> regexIdToTokType = m_language->getSemantic()->get_token_type_regex_index_to_token_type();

    std::string prefix;

    // Unified parsing using a char iterator (loop over all regex)
    auto unifiedParsing = [&](auto& it) -> auto
    {
        int i = 0;
        for (auto eachRegexIt = regex.cbegin(); eachRegexIt != regex.cend(); eachRegexIt++)
        {
            i++;
            std::smatch sm;
            auto match = std::regex_search(it, chars.cend(), sm, *eachRegexIt);

            if (match)
            {
                auto matchedTokenString = sm.str();
                auto matchedTokenType   = regexIdToTokType[std::distance(regex.cbegin(), eachRegexIt)];

                if (matchedTokenType != TokenType_Ignore)
                {
                    Token* newToken = m_token_ribbon.push(matchedTokenType, matchedTokenString, std::distance(chars.cbegin(), it));
                    LOG_VERBOSE("Parser", "tokenize <word>%s</word>\n", matchedTokenString.c_str() )

                    // If a we have so prefix tokens we copy them to the newToken prefixes.
                    if ( !prefix.empty() )
                    {
                        newToken->m_prefix = prefix;
                        prefix.clear();
                    }

                }
                else if ( !m_token_ribbon.empty()  )
                {
                    Token& lastToken = m_token_ribbon.tokens.back();
                    lastToken.m_suffix.append(matchedTokenString);
                    LOG_VERBOSE("Parser", "append ignored <word>%s</word> to <word>%s</word>\n", matchedTokenString.c_str(), lastToken.m_word.c_str() )
                }
                else
                {
                    prefix.append(matchedTokenString);
                }

                // advance iterator to the end of the str
                std::advance(it, matchedTokenString.length());
                return true;
            }
        }
        return false;
    };

    auto currTokIt = chars.cbegin();
	while(currTokIt != chars.cend())
	{
		if (!unifiedParsing(currTokIt))
		{
		    LOG_VERBOSE("Parser", "tokenize " KO ", unable to tokenize at index %i\n", (int)std::distance(chars.cbegin(), currTokIt) )
			return false;
		}
	}

    LOG_VERBOSE("Parser", "tokenize " OK " \n" )
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
    std::string identifier;
    const Token* token_0 = m_token_ribbon.eatToken();
    const Token* token_1 = m_token_ribbon.eatToken();
    if (token_0->m_type == TokenType_Identifier &&
        token_1->m_type == TokenType_OpenBracket)
    {
        identifier = token_0->m_word;
        LOG_VERBOSE("Parser", "parse function call... " OK " regular function pattern detected.\n")
    }
    else // Try to parse operator like (ex: operator==(..,..))
    {
        const Token* token_2 = m_token_ribbon.eatToken(); // eat a "supposed open bracket>

        if (token_0->m_type == TokenType_Identifier && token_0->m_word == m_language->getSemantic()
                                                                                  ->token_type_to_string(
                                                                                          TokenType_KeywordOperator /* TODO: TokenType_Keyword + word="operator" */) &&
            token_1->m_type == TokenType_Operator &&
            token_2->m_type == TokenType_OpenBracket)
        {
            // ex: "operator" + ">=>
            identifier = token_0->m_word + token_1->m_word;
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
    FunctionSignature signature(identifier, Type_Any);

    bool parsingError = false;
    while (!parsingError && m_token_ribbon.canEat() && m_token_ribbon.peekToken()->m_type != TokenType_CloseBracket)
    {

        if (auto member = parse_expression())
        {
            args.push_back(member); // store argument as member (already parsed)
            signature.push_arg(member->getType());  // add a new argument type to the proto.
            m_token_ribbon.eatToken(TokenType_Separator);
        }
        else
        {
            parsingError = true;
        }
    }

    // eat "close bracket supposed" token
    if ( !m_token_ribbon.eatToken(TokenType_CloseBracket) )
    {
        LOG_ERROR("Parser", "parse function call... " KO " abort, close parenthesis expected. \n")
        rollback_transaction();
        return nullptr;
    }


    // Find the prototype in the language library
    auto fct = m_language->findFunction(&signature);

    if (fct != nullptr)
    {
        auto node = m_graph->newFunction(fct);

        auto connectArg = [&](size_t _argIndex) -> void
        { // lambda to connect input member to node for a specific argument index.

            auto arg = args.at(_argIndex);
            auto memberName = fct
                    ->get_signature()
                    ->get_args()
                    .at(_argIndex)
                    .m_name;

            m_graph->connect(arg, node->getProps()->get(memberName.c_str()));
        };

        for (size_t argIndex = 0; argIndex < fct->get_signature()->get_arg_count(); argIndex++)
        {
            connectArg(argIndex);
        }

        commit_transaction();
        LOG_VERBOSE("Parser", "parse function call... " OK "\n")

        return node->getProps()->get("result");

    }

    std::string signature_str;
    m_language->getSerializer()->serialize(signature_str, &signature);
    LOG_ERROR("Parser", "parse function call... " KO " abort, reason: %s not found.\n", signature_str.c_str() )
    rollback_transaction();
    return nullptr;
}

ScopedCodeBlockNode *Parser::get_current_scope()
{
    // TODO: implement. For now return only the global scope
    return m_graph->getProgram();
}

ConditionalStructNode * Parser::parse_conditional_structure()
{
    LOG_VERBOSE("Parser", "try to parse conditional structure...\n")
    start_transaction();

    auto condStruct = m_graph->newConditionalStructure();

    if ( m_token_ribbon.eatToken(TokenType_KeywordIf))
    {
        condStruct->set_token_if(m_token_ribbon.getEaten());

        auto condition = parse_parenthesis_expression();

        if ( condition)
        {
            m_graph->connect(condition, condStruct->get_condition() );
            if ( ScopedCodeBlockNode* scopeIf = parse_scope() )
            {
                m_graph->connect(scopeIf, condStruct, RelationType::IS_CHILD_OF);

                if ( m_token_ribbon.eatToken(TokenType_KeywordElse))
                {
                    condStruct->set_token_else(m_token_ribbon.getEaten());

                    if ( ScopedCodeBlockNode* scopeElse = parse_scope() )
                    {
                        m_graph->connect(scopeElse, condStruct, RelationType::IS_CHILD_OF);
                        commit_transaction();
                        LOG_VERBOSE("Parser", "parse IF {...} ELSE {...} block... " OK "\n")
                        return condStruct;
                    }

                    if ( ConditionalStructNode* elseIfCondStruct = parse_conditional_structure() )
                    {
						m_graph->connect(elseIfCondStruct, condStruct, RelationType::IS_CHILD_OF);
                        commit_transaction();
						LOG_VERBOSE("Parser", "parse IF {...} ELSE IF {...} block... " OK "\n")
						return condStruct;
                    }

                    LOG_VERBOSE("Parser", "parse IF {...} ELSE {...} block... " KO "\n")
                    m_graph->deleteNode(scopeIf);

                } else {
                    commit_transaction();
                    LOG_VERBOSE("Parser", "parse IF {...} block... " OK "\n")
                    return condStruct;
                }
            }
            else
            {
                LOG_VERBOSE("Parser", "parse IF {...} block... " KO "\n")
            }
        }
    }

    m_graph->deleteNode(condStruct);
    rollback_transaction();
    return nullptr;
}

ForLoopNode* Parser::parse_for_loop()
{
    start_transaction();

    Token* token_for = m_token_ribbon.eatToken(TokenType_KeywordFor);

    if( token_for != nullptr )
    {
        LOG_VERBOSE("Parser", "parse FOR (...) block...\n")
        Token* open_bracket = m_token_ribbon.eatToken(TokenType_OpenBracket);
        if( !open_bracket )
        {
            LOG_ERROR("Parser", "Unable to find open bracket after for keyword.\n")
            rollback_transaction();
            return nullptr;
        }

        Member* init_instr = parse_expression();
        if ( !init_instr )
        {
            LOG_ERROR("Parser", "Unable to find initial instruction.\n")
            rollback_transaction();
            return nullptr;
        }

        if ( !m_token_ribbon.eatToken(TokenType_EndOfInstruction) )
        {
            LOG_ERROR("Parser", "Unable to find end of instruction after initial instruction.\n")
            rollback_transaction();
            return nullptr;
        }

        Member* cond_instr = parse_expression();
        if ( !cond_instr )
        {
            LOG_ERROR("Parser", "Unable to find condition instruction.\n")
            rollback_transaction();
            return nullptr;
        }

        if ( !m_token_ribbon.eatToken(TokenType_EndOfInstruction) )
        {
            LOG_ERROR("Parser", "Unable to find end of instruction after condition.\n")
            rollback_transaction();
            return nullptr;
        }

        Member* iter_instr = parse_expression();
        if ( !iter_instr )
        {
            LOG_ERROR("Parser", "Unable to find iterative instruction.\n")
            rollback_transaction();
            return nullptr;
        }

        Token* close_bracket = m_token_ribbon.eatToken(TokenType_CloseBracket);
        if ( !close_bracket )
        {
            LOG_ERROR("Parser", "Unable to find close bracket after iterative instruction.\n")
            rollback_transaction();
            return nullptr;
        }

        ForLoopNode* for_loop_node = m_graph->new_for_loop_node();
        for_loop_node->set_token_for( token_for );

        m_graph->connect(init_instr, for_loop_node->get_init_expr() );
        m_graph->connect(cond_instr, for_loop_node->get_condition() );
        m_graph->connect(iter_instr, for_loop_node->get_iter_expr() );

        ScopedCodeBlockNode* for_scope = parse_scope();
        if ( !for_scope )
        {
            LOG_ERROR("Parser", "Unable to parse a scope after for(...).\n")
            rollback_transaction();
            return nullptr;
        }

        m_graph->connect(for_scope, for_loop_node, RelationType::IS_NEXT_OF );

        commit_transaction();
        return for_loop_node;

    }
    else
    {
        rollback_transaction();
        return nullptr;
    }
}

Member *Parser::parse_variable_declaration()
{

    if( !m_token_ribbon.canEat(2))
        return nullptr;

    start_transaction();

    Token* typeTok = m_token_ribbon.eatToken();
    Token* identifierTok = m_token_ribbon.eatToken();

    if(Token::isType(typeTok->m_type) && identifierTok->m_type == TokenType_Identifier )
    {
        Type type = m_language->getSemantic()->token_type_to_type(typeTok->m_type);
        VariableNode* variable = m_graph->newVariable(type, identifierTok->m_word, this->get_current_scope());
        variable->setTypeToken( typeTok );
        variable->setIdentifierToken( identifierTok );

        variable->value()->setType(m_language->getSemantic()->token_type_to_type(typeTok->m_type));
        variable->value()->setSourceToken(identifierTok); // we also pass it to the member, this one will be modified my connections

        // try to parse assignment
        auto assignmentTok = m_token_ribbon.eatToken(TokenType_Operator);
        if ( assignmentTok && assignmentTok->m_word == "=" )
        {
            if( auto value = parse_expression() )
            {
                m_graph->connect(value, variable->value());
                variable->setAssignmentOperatorToken( assignmentTok );
            }
            else
            {
                LOG_ERROR("Parser", "Unable to parse expression to assign %s\n", identifierTok->m_word.c_str())
                rollback_transaction();
                m_graph->deleteNode(variable);
                return nullptr;
            }
        }

        commit_transaction();
        return variable->value();
    }

    rollback_transaction();
    return nullptr;
}
