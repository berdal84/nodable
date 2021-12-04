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
#include <nodable/ProgramNode.h>
#include <nodable/LiteralNode.h>
#include <nodable/VariableNode.h>
#include <nodable/InvokableComponent.h>

using namespace Nodable;

void Parser::rollbackTransaction()
{
    tokenRibbon.rollbackTransaction();
}

void Parser::startTransaction()
{
    tokenRibbon.startTransaction();
}

void Parser::commitTransaction()
{
    tokenRibbon.commitTransaction();
}

bool Parser::expressionToGraph(const std::string& _code,
                               GraphNode* _graphNode )
{
    graph = _graphNode;
    tokenRibbon.clear();

    LOG_VERBOSE("Parser", "Trying to evaluate evaluated: <expr>%s</expr>\"\n", _code.c_str() )

    std::istringstream iss(_code);
    std::string line;
    std::string eol;
    language->getSerializer()->serialize(eol, TokenType_EndOfLine);

    size_t lineCount = 0;
    while (std::getline(iss, line, eol[0] ))
    {
        if ( lineCount != 0 && !tokenRibbon.tokens.empty() )
        {
            Token* lastToken = &tokenRibbon.tokens.back();
            lastToken->m_suffix.append(eol);
        }

        if (!tokenizeExpressionString(line))
        {
            LOG_WARNING("Parser", "Unable to parse code due to unrecognized tokens.\n")
            return false;
        }

        lineCount++;
    }

	if (tokenRibbon.empty() )
    {
        LOG_MESSAGE("Parser", "Empty code. Nothing to evaluate.\n")
        return false;
    }

	if (!isSyntaxValid())
	{
		LOG_WARNING("Parser", "Unable to parse code due to syntax error.\n")
		return false;
	}

	CodeBlockNode* program = parseProgram();

	if (program == nullptr)
	{
		LOG_WARNING("Parser", "Unable to parse main scope due to abstract syntax tree failure.\n")
		return false;
	}

    if ( tokenRibbon.canEat() )
    {
        graph->clear();
        LOG_ERROR("Parser", "Unable to evaluate the full expression.\n")
        return false;
    }

    // We unset dirty, since we did a lot of connections but we don't want any update now
    auto& nodes = graph->getNodeRegistry();
    for(auto eachNode : nodes )
        eachNode->setDirty(false);

	LOG_MESSAGE("Parser", "Graph well updated.\n", _code.c_str() )
	LOG_VERBOSE("Parser", "Expression evaluated: <expr>%s</expr>\"\n", _code.c_str() )
	return true;
}

Type Parser::getTokenLiteralType(const Nodable::Token *_token) const
{
    Type type = Type_Unknown;

    const std::vector<std::regex> regex            = language->getSemantic()->get_type_regex();
    const std::vector<Type>       regex_id_to_type = language->getSemantic()->get_type_regex_index_to_type();

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

bool Parser::parseBool(const std::string& _str)
{
    return _str == std::string("true");
}

std::string Parser::parseString(const std::string& _str)
{
    NODABLE_ASSERT(_str.size() >= 2);
    NODABLE_ASSERT(_str[0] == '\"');
    NODABLE_ASSERT(_str[_str.size()-1] == '\"');
    return std::string(++_str.cbegin(), --_str.cend());
}

double Parser::parseDouble(const std::string& _str)
{
    return stod(_str);
}

Member* Parser::tokenToMember(Token* _token)
{
	Member* result = nullptr;

	switch (_token->m_type)
	{

	    case TokenType_Literal:
        {
            Type type = getTokenLiteralType(_token);
            LiteralNode* literal = graph->newLiteral(type);

            switch ( type ) {
                case Type_String: literal->set_value( parseString(_token->m_word) ); break;
                case Type_Double: literal->set_value( parseDouble(_token->m_word) ); break;
                case Type_Boolean: literal->set_value( parseBool(_token->m_word)  ); break;
                default: {}
            }

            result = literal->get_value();
            result->setSourceToken(_token);
            break;
        }

		case TokenType_Identifier:
		{
			VariableNode* variable = graph->findVariable(_token->m_word);

			if (variable == nullptr) {
                LOG_WARNING("Parser", "Unable to find declaration for %s \n", _token->m_word.c_str())
                variable = graph->newVariable(Type_Any, _token->m_word, getCurrentScope() );
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

Member* Parser::parseBinaryOperationExpression(unsigned short _precedence, Member* _left) {

    assert(_left != nullptr);

    LOG_VERBOSE("Parser", "parse binary operation expr...\n")
    LOG_VERBOSE("Parser", "%s \n", tokenRibbon.toString().c_str())

	Member* result = nullptr;

	if ( !tokenRibbon.canEat(2))
	{
		LOG_VERBOSE("Parser", "parse binary operation expr...... " KO " (not enought tokens)\n")
		return nullptr;
	}

	startTransaction();
	Token* operatorToken = tokenRibbon.eatToken();
	const Token* token2 = tokenRibbon.peekToken();

	// Structure check
	const bool isValid = _left != nullptr &&
                         operatorToken->m_type == TokenType_Operator &&
                         token2->m_type != TokenType_Operator;

	if (!isValid)
	{
	    rollbackTransaction();
		LOG_VERBOSE("Parser", "parse binary operation expr... " KO " (Structure)\n")
		return nullptr;
	}

	// Precedence check
	const auto currentOperatorPrecedence = language->findOperator(operatorToken->m_word)->getPrecedence();

	if (currentOperatorPrecedence <= _precedence &&
	    _precedence > 0u) { // always update the first operation if they have the same precedence or less.
		LOG_VERBOSE("Parser", "parse binary operation expr... " KO " (Precedence)\n")
		rollbackTransaction();
		return nullptr;
	}


	// Parse right expression
	auto right = parseExpression(currentOperatorPrecedence, nullptr );

	if (!right)
	{
		LOG_VERBOSE("Parser", "parseBinaryOperationExpression... " KO " (right expression is nullptr)\n")
		rollbackTransaction();
		return nullptr;
	}

	// Create a function signature according to ltype, rtype and operator word
	const FunctionSignature* signature = language->createBinOperatorSignature(Type_Any, operatorToken->m_word, _left->getType(), right->getType());
	auto matchingOperator = language->findOperator(signature);
    delete signature;

	if ( matchingOperator != nullptr )
	{
		auto binOpNode = graph->newBinOp(matchingOperator);
        auto computeComponent = binOpNode->getComponent<InvokableComponent>();
        computeComponent->set_source_token(operatorToken);

        graph->connect(_left, computeComponent->get_l_handed_val());
        graph->connect(right, computeComponent->get_r_handed_val());
		result = binOpNode->getProps()->get("result");

        commitTransaction();
        LOG_VERBOSE("Parser", "parse binary operation expr... " OK "\n")

        return result;
    }
    else
    {
        LOG_VERBOSE("Parser", "parse binary operation expr... " KO " (unable to find operator prototype)\n")
        rollbackTransaction();
        return nullptr;
    }
}

Member* Parser::parseUnaryOperationExpression(unsigned short _precedence)
{
	LOG_VERBOSE("Parser", "parseUnaryOperationExpression...\n")
	LOG_VERBOSE("Parser", "%s \n", tokenRibbon.toString().c_str())

	if (!tokenRibbon.canEat(2) )
	{
		LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (not enough tokens)\n")
		return nullptr;
	}

	startTransaction();
	Token* operatorToken = tokenRibbon.eatToken();

	// Check if we get an operator first
	if (operatorToken->m_type != TokenType_Operator)
	{
	    rollbackTransaction();
		LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (operator not found)\n")
		return nullptr;
	}

	// Parse expression after the operator
	Member* value = parseAtomicExpression();

  if ( value == nullptr )
  {
    value = parseParenthesisExpression();
  }

  if ( value == nullptr )
  {
		LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (right expression is nullptr)\n")
		rollbackTransaction();
		return nullptr;
	}

	// Create a function signature
	auto signature = language->createUnaryOperatorSignature(Type_Any, operatorToken->m_word, value->getType() );
	auto matchingOperator = language->findOperator(signature);

	if (matchingOperator != nullptr)
	{
		auto unaryOpNode = graph->newUnaryOp(matchingOperator);
        auto computeComponent = unaryOpNode->getComponent<InvokableComponent>();
        computeComponent->set_source_token(operatorToken);

        graph->connect(value, computeComponent->get_l_handed_val());
        Member* result = unaryOpNode->getProps()->get("result");

		LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " OK "\n")
        commitTransaction();

		return result;
	}
	else
	{
		LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (unrecognysed operator)\n")
		rollbackTransaction();
		return nullptr;
	}
}

Member* Parser::parseAtomicExpression()
{
	LOG_VERBOSE("Parser", "parse atomic expr... \n")

	if ( !tokenRibbon.canEat() )
	{
		LOG_VERBOSE("Parser", "parse atomic expr... " KO "(not enough tokens)\n")
		return nullptr;
	}

	startTransaction();
	Token* token = tokenRibbon.eatToken();
	if (token->m_type == TokenType_Operator)
	{
		LOG_VERBOSE("Parser", "parse atomic expr... " KO "(token is an operator)\n")
		rollbackTransaction();
		return nullptr;
	}

	auto result = tokenToMember(token);
	if( result != nullptr)
    {
	    commitTransaction();
        LOG_VERBOSE("Parser", "parse atomic expr... " OK "\n")
    }
	else
    {
        rollbackTransaction();
        LOG_VERBOSE("Parser", "parse atomic expr... " KO " (result is nullptr)\n")
	}

	return result;
}

Member* Parser::parseParenthesisExpression()
{
	LOG_VERBOSE("Parser", "parse parenthesis expr...\n")
	LOG_VERBOSE("Parser", "%s \n", tokenRibbon.toString().c_str())

	if ( !tokenRibbon.canEat() )
	{
		LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " no enough tokens.\n")
		return nullptr;
	}

	startTransaction();
	const Token* currentToken = tokenRibbon.eatToken();
	if (currentToken->m_type != TokenType_OpenBracket)
	{
		LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " open bracket not found.\n")
		rollbackTransaction();
		return nullptr;
	}

    Member* result = parseExpression();
	if (result)
	{
        const Token* token = tokenRibbon.eatToken();
		if (token->m_type != TokenType_CloseBracket )
		{
			LOG_VERBOSE("Parser", "%s \n", tokenRibbon.toString().c_str())
			LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " ( \")\" expected instead of %s )\n", token->m_word.c_str() )
            rollbackTransaction();
		}
		else
        {
			LOG_VERBOSE("Parser", "parse parenthesis expr..." OK  "\n")
            commitTransaction();
		}
	}
	else
    {
        LOG_VERBOSE("Parser", "parse parenthesis expr..." KO ", expression in parenthesis is nullptr.\n")
	    rollbackTransaction();
	}
	return result;
}

InstructionNode* Parser::parseInstruction()
{
    startTransaction();

    Member* parsedExpression = parseExpression();

    if ( parsedExpression == nullptr )
    {
       LOG_VERBOSE("Parser", "parse instruction " KO " (parsed is nullptr)\n")
       rollbackTransaction();
       return nullptr;
    }

    auto instruction = graph->newInstruction();

    if ( tokenRibbon.canEat() )
    {
        Token* expectedEOI = tokenRibbon.eatToken(TokenType_EndOfInstruction);
        if ( expectedEOI )
        {
            instruction->setEndOfInstrToken( expectedEOI );
        }
        else
        {
            LOG_VERBOSE("Parser", "parse instruction " KO " (end of instruction not found)\n")
            rollbackTransaction();
            return nullptr;
        }
    }

    graph->connect(parsedExpression, instruction);

    LOG_VERBOSE("Parser", "parse instruction " OK "\n")
    commitTransaction();
    return instruction;
}

CodeBlockNode* Parser::parseProgram()
{
    startTransaction();
    auto scope = graph->newProgram();
    if(CodeBlockNode* block = parseCodeBlock())
    {
        graph->connect(block, scope, RelationType::IS_CHILD_OF);
        commitTransaction();
        return block;
    }
    else
    {
        graph->clear();
        rollbackTransaction();
        return nullptr;
    }
}

ScopedCodeBlockNode* Parser::parseScope()
{
    startTransaction();

    if ( !tokenRibbon.eatToken(TokenType_BeginScope))
    {
        rollbackTransaction();
        return nullptr;
    }

    auto scope = graph->newScopedCodeBlock();
    scope->setBeginScopeToken( tokenRibbon.getEaten() );

    if ( auto block = parseCodeBlock() )
    {
        graph->connect(block, scope, RelationType::IS_CHILD_OF);
    }

    if ( !tokenRibbon.eatToken(TokenType_EndScope))
    {
        graph->deleteNode(scope);
        rollbackTransaction();
        return nullptr;
    }
    scope->setEndScopeToken( tokenRibbon.getEaten() );

    commitTransaction();
    return scope;
}

CodeBlockNode* Parser::parseCodeBlock()
{
    startTransaction();

    auto block = graph->newCodeBlock();

    bool stop = false;

//    Node* previous = block;
    while(tokenRibbon.canEat() && !stop )
    {
        if ( InstructionNode* instruction = parseInstruction() )
        {
            graph->connect(instruction, block, RelationType::IS_CHILD_OF);
        }
        else if ( ScopedCodeBlockNode* scope = parseScope() )
        {
            graph->connect(scope, block, RelationType::IS_CHILD_OF);
        }
        else if ( ConditionalStructNode* condStruct = parseConditionalStructure() )
        {
            graph->connect(condStruct, block, RelationType::IS_CHILD_OF);
        }
        else
        {
            stop = true;
        }
    }

    if ( block->getChildren().empty() )
    {
        graph->deleteNode(block);
        rollbackTransaction();
        return nullptr;
    }
    else
    {
        commitTransaction();
        return block;
    }
}

Member* Parser::parseExpression(unsigned short _precedence, Member* _leftOverride)
{
	LOG_VERBOSE("Parser", "parse expr...\n")
	LOG_VERBOSE("Parser", "%s \n", tokenRibbon.toString().c_str())

	if ( !tokenRibbon.canEat() )
	{
		LOG_VERBOSE("Parser", "parse expr..." KO " (unable to eat a single token)\n")
        return _leftOverride;
	}

	/*
		Get the left handed operand
	*/
	Member* left = _leftOverride;
  if ( left == nullptr ) left = parseParenthesisExpression();
  if ( left == nullptr ) left = parseUnaryOperationExpression(_precedence);
  if ( left == nullptr ) left = parseFunctionCall();
  if ( left == nullptr ) left = parseVariableDecl();
  if ( left == nullptr ) left = parseAtomicExpression();

	if ( !tokenRibbon.canEat() )
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
		auto binResult = parseBinaryOperationExpression(_precedence, left);

		if (binResult)
		{
			LOG_VERBOSE("Parser", "parse expr... right parsed, recursive call\n")
			result = parseExpression(_precedence, binResult);
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

bool Parser::isSyntaxValid()
{
    // TODO: optimization: is this function really useful ? It check only few things.
    //                     The parsing steps that follow (parseProgram) is doing a better check, by looking to what exist in the Language.
	bool success   = true;
	auto currTokIt = tokenRibbon.tokens.begin();
	short int openedParenthesisCount = 0;

	while(currTokIt != tokenRibbon.tokens.end() && success)
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

bool Parser::tokenizeExpressionString(const std::string& _expression)
{
    /* get expression chars */
    auto chars = _expression;

    /* shortcuts to language members */
    const std::vector<std::regex> regex           = language->getSemantic()->get_token_type_regex();
    const std::vector<TokenType> regexIdToTokType = language->getSemantic()->get_token_type_regex_index_to_token_type();

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
                    Token* newToken = tokenRibbon.push(matchedTokenType, matchedTokenString, std::distance(chars.cbegin(), it));
                    LOG_VERBOSE("Parser", "tokenize <word>%s</word>\n", matchedTokenString.c_str() )

                    // If a we have so prefix tokens we copy them to the newToken prefixes.
                    if ( !prefix.empty() )
                    {
                        newToken->m_prefix = prefix;
                        prefix.clear();
                    }

                }
                else if ( !tokenRibbon.empty()  )
                {
                    Token& lastToken = tokenRibbon.tokens.back();
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

Member* Parser::parseFunctionCall()
{
    LOG_VERBOSE("Parser", "parse function call...\n")

    // Check if the minimum token count required is available ( 0: identifier, 1: open parenthesis, 2: close parenthesis)
    if (!tokenRibbon.canEat(3))
    {
        LOG_VERBOSE("Parser", "parse function call... " KO " aborted, not enough tokens.\n")
        return nullptr;
    }

    startTransaction();

    // Try to parse regular function: function(...)
    std::string identifier;
    const Token* token_0 = tokenRibbon.eatToken();
    const Token* token_1 = tokenRibbon.eatToken();
    if (token_0->m_type == TokenType_Identifier &&
        token_1->m_type == TokenType_OpenBracket)
    {
        identifier = token_0->m_word;
        LOG_VERBOSE("Parser", "parse function call... " OK " regular function pattern detected.\n")
    }
    else // Try to parse operator like (ex: operator==(..,..))
    {
        const Token* token_2 = tokenRibbon.eatToken(); // eat a "supposed open bracket>

        if (token_0->m_type == TokenType_Identifier && token_0->m_word == language->getSemantic()
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
            rollbackTransaction();
            return nullptr;
        }
    }
    std::vector<Member *> args;

    // Declare a new function prototype
    FunctionSignature signature(identifier, Type_Any);

    bool parsingError = false;
    while (!parsingError && tokenRibbon.canEat() && tokenRibbon.peekToken()->m_type != TokenType_CloseBracket)
    {

        if (auto member = parseExpression())
        {
            args.push_back(member); // store argument as member (already parsed)
            signature.pushArg( member->getType() );  // add a new argument type to the proto.
            tokenRibbon.eatToken(TokenType_Separator);
        }
        else
        {
            parsingError = true;
        }
    }

    // eat "close bracket supposed" token
    if ( !tokenRibbon.eatToken(TokenType_CloseBracket) )
    {
        LOG_ERROR("Parser", "parse function call... " KO " abort, close parenthesis expected. \n")
        rollbackTransaction();
        return nullptr;
    }


    // Find the prototype in the language library
    auto fct = language->findFunction(&signature);

    if (fct != nullptr)
    {
        auto node = graph->newFunction(fct);

        auto connectArg = [&](size_t _argIndex) -> void
        { // lambda to connect input member to node for a specific argument index.

            auto arg = args.at(_argIndex);
            auto memberName = fct
                    ->getSignature()
                    ->getArgs()
                    .at(_argIndex)
                    .name;

            graph->connect(arg, node->getProps()->get(memberName.c_str()));
        };

        for (size_t argIndex = 0; argIndex < fct->getSignature()->getArgCount(); argIndex++)
        {
            connectArg(argIndex);
        }

        commitTransaction();
        LOG_VERBOSE("Parser", "parse function call... " OK "\n")

        return node->getProps()->get("result");

    }

    std::string signature_str;
    language->getSerializer()->serialize(signature_str, &signature);
    LOG_ERROR("Parser", "parse function call... " KO " abort, reason: %s not found.\n", signature_str.c_str() )
    rollbackTransaction();
    return nullptr;
}

ScopedCodeBlockNode *Parser::getCurrentScope()
{
    // TODO: implement. For now return only the global scope
    return graph->getProgram();
}

ConditionalStructNode * Parser::parseConditionalStructure()
{
    LOG_VERBOSE("Parser", "try to parse conditional structure...\n")
    startTransaction();

    auto condStruct = graph->newConditionalStructure();

    if ( tokenRibbon.eatToken(TokenType_KeywordIf))
    {
        condStruct->setTokenIf(tokenRibbon.getEaten());

        auto condition = parseParenthesisExpression();

        if ( condition)
        {
            graph->connect(condition, condStruct->getCondition() );
            if ( ScopedCodeBlockNode* scopeIf = parseScope() )
            {
                graph->connect(scopeIf, condStruct, RelationType::IS_CHILD_OF);

                if ( tokenRibbon.eatToken(TokenType_KeywordElse))
                {
                    condStruct->setTokenElse( tokenRibbon.getEaten() );

                    if ( ScopedCodeBlockNode* scopeElse = parseScope() )
                    {
                        graph->connect(scopeElse, condStruct, RelationType::IS_CHILD_OF);
                        commitTransaction();
                        LOG_VERBOSE("Parser", "parse IF {...} ELSE {...} block... " OK "\n")
                        return condStruct;
                    }
                    else if ( ConditionalStructNode* elseIfCondStruct = parseConditionalStructure() )
                    {
						graph->connect(elseIfCondStruct, condStruct, RelationType::IS_CHILD_OF);
						commitTransaction();
						LOG_VERBOSE("Parser", "parse IF {...} ELSE IF {...} block... " OK "\n")
						return condStruct;
                    }

                    LOG_VERBOSE("Parser", "parse IF {...} ELSE {...} block... " KO "\n")
                    graph->deleteNode(scopeIf);

                } else {
                    commitTransaction();
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

    graph->deleteNode(condStruct);
    rollbackTransaction();
    return nullptr;
}

Member *Parser::parseVariableDecl()
{

    if( !tokenRibbon.canEat(2))
        return nullptr;

    startTransaction();

    Token* typeTok = tokenRibbon.eatToken();
    Token* identifierTok = tokenRibbon.eatToken();

    if(Token::isType(typeTok->m_type) && identifierTok->m_type == TokenType_Identifier )
    {
        Type type = language->getSemantic()->token_type_to_type(typeTok->m_type);
        VariableNode* variable = graph->newVariable(type, identifierTok->m_word, this->getCurrentScope());
        variable->setTypeToken( typeTok );
        variable->setIdentifierToken( identifierTok );

        variable->value()->setType(language->getSemantic()->token_type_to_type(typeTok->m_type));
        variable->value()->setSourceToken(identifierTok); // we also pass it to the member, this one will be modified my connections

        // try to parse assignment
        auto assignmentTok = tokenRibbon.eatToken(TokenType_Operator);
        if ( assignmentTok && assignmentTok->m_word == "=" )
        {
            if( auto value = parseExpression() )
            {
                graph->connect(value, variable->value());
                variable->setAssignmentOperatorToken( assignmentTok );
            }
            else
            {
                LOG_ERROR("Parser", "Unable to parse expression to assign %s\n", identifierTok->m_word.c_str())
                rollbackTransaction();
                graph->deleteNode(variable);
                return nullptr;
            }
        }

        commitTransaction();
        return variable->value();
    }

    rollbackTransaction();
    return nullptr;
}
