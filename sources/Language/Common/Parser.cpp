#include "Parser.h"
#include "Log.h"          // for LOG_VERBOSE(...)
#include "Member.h"
#include "Container.h"
#include "Variable.h"
#include "Wire.h"
#include "Language.h"
#include "Log.h"
#include <regex>
#include <algorithm>

using namespace Nodable;

bool Parser::evalExprIntoContainer(const std::string& _expression,
                                   Container* _container )
{
    tokens.clear();
    container = _container;

	if (!tokenizeExpressionString(_expression))
	{
		LOG_WARNING("Parser", "Unable to parse expression due to unrecognized tokens.\n");
       return false;
	}

	if (tokens.empty() )
    {
        LOG_MESSAGE("Parser", "Nothing to evaluate.\n");
        return false;
    }

	if (!isSyntaxValid())
	{
		LOG_WARNING("Parser", "Unable to parse expression due to syntax error.\n");
		return false;
	}

	Member* resultValue = parseRootExpression();
	if (resultValue == nullptr)
	{
		LOG_WARNING("Parser", "Unable to parse expression due to abstract syntax tree failure.\n");
		return false;
	}

	Variable* result = container->newResult();
	container->tryToRestoreResultNodePosition();

	// If the value has no owner, we simply set the variable value
	if (resultValue->getOwner() == nullptr)
	{
        result->set(resultValue);
        delete resultValue;
    }
	else // we connect resultValue with resultVariable.value
    {
        Node::Connect(resultValue, result->value());
    }

	LOG_MESSAGE( "Parser", "Expression evaluated: %s\n", _expression.c_str() );
	return true;
}

Member* Parser::tokenToMember(const Token& _token) {


	Member* result = nullptr;

	switch (_token.type)
	{

		case TokenType::Boolean:
		{
			result = new Member(nullptr);
			const bool value = _token.word == "true";
			result->set(value);
			break;
		}

		case TokenType::Symbol:
		{
			auto context = container;
			Variable* variable = context->findVariable(_token.word);

			if (variable == nullptr)
				variable = context->newVariable(_token.word);

			NODABLE_ASSERT(variable != nullptr);
			NODABLE_ASSERT(variable->value() != nullptr);

			result = variable->value();

			break;
		}

		case TokenType::Double: {
			result = new Member(nullptr);
			const double number = std::stod(_token.word);
			result->set(number);
			break;
		}

		case TokenType::String: {
			result = new Member(nullptr);
			result->set(_token.word);
			break;
		}

	    default:
	        assert("This TokenType is not handled by this method.");

	}

	return result;
}

Member* Parser::parseBinaryOperationExpression(unsigned short _precedence, Member* _left) {

    assert(_left != nullptr);

    LOG_VERBOSE("Parser", "parse binary operation expr...\n");
    LOG_VERBOSE("Parser", "%s \n", tokens.toString().c_str());

	Member* result = nullptr;

	if ( !tokens.canEat(2))
	{
		LOG_VERBOSE("Parser", "parse binary operation expr...... " KO " (not enought tokens)\n");
		return nullptr;
	}

	tokens.startTransaction();
	const Token& token1 = tokens.eatToken();
	const Token& token2 = tokens.peekToken();

	// Structure check
	const bool isValid = _left != nullptr &&
			             token1.type == TokenType::Operator &&
			             token2.type != TokenType::Operator;

	if (!isValid)
	{
	    tokens.rollbackTransaction();
		LOG_VERBOSE("Parser", "parse binary operation expr... " KO " (Structure)\n");
		return nullptr;
	}

	// Precedence check
	const auto currentOperatorPrecedence = language->findOperator(token1.word)->precedence;

	if (currentOperatorPrecedence <= _precedence &&
	    _precedence > 0u) { // always eval the first operation if they have the same precedence or less.
		LOG_VERBOSE("Parser", "parse binary operation expr... " KO " (Precedence)\n");
		tokens.rollbackTransaction();
		return nullptr;
	}


	// Parse right expression
	auto right = parseExpression(currentOperatorPrecedence, nullptr );

	if (!right)
	{
		LOG_VERBOSE("Parser", "parseBinaryOperationExpression... " KO " (right expression is nullptr)\n");
		tokens.rollbackTransaction();
		return nullptr;
	}

	// Create a function signature according to ltype, rtype and operator word
	auto signature        = language->createBinOperatorSignature(Type::Any, token1.word, _left->getType(), right->getType());
	auto matchingOperator = language->findOperator(signature);

	if ( matchingOperator != nullptr )
	{
		auto binOpNode = container->newBinOp( matchingOperator);

		// Connect the Left Operand :
		//---------------------------
		if (_left->getOwner() == nullptr)
		{
            binOpNode->set("lvalue", _left);
            delete _left;
        }
		else
        {
			Node::Connect( _left, binOpNode->get("lvalue"));
        }

		// Connect the Right Operand :

		if (right->getOwner() == nullptr)
		{
            binOpNode->set("rvalue", right);
            delete right;
        }
		else
        {
			Node::Connect(right, binOpNode->get("rvalue"));
        }

		// Set the left !
		result = binOpNode->get("result");

        tokens.commitTransaction();
        LOG_VERBOSE("Parser", "parse binary operation expr... " OK "\n");

        return result;
    }
    else
    {
        LOG_VERBOSE("Parser", "parse binary operation expr... " KO " (unable to find operator prototype)\n");
        tokens.rollbackTransaction();
        return nullptr;
    }
}

Member* Parser::parseUnaryOperationExpression(unsigned short _precedence)
{
	LOG_VERBOSE("Parser", "parseUnaryOperationExpression...\n");
	LOG_VERBOSE("Parser", "%s \n", tokens.toString().c_str());

	if (!tokens.canEat(2) )
	{
		LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (not enough tokens)\n");
		return nullptr;
	}

	tokens.startTransaction();
	const Token& operatorToken = tokens.eatToken();

	// Check if we get an operator first
	if (operatorToken.type != TokenType::Operator)
	{
	    tokens.rollbackTransaction();
		LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (operator not found)\n");
		return nullptr;
	}

	// Parse expression after the operator
	auto precedence = language->findOperator(operatorToken.word)->precedence;
	Member* value = nullptr;

	     if ( value = parseAtomicExpression() );
	else if ( value = parseParenthesisExpression() );
	else
	{
		LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (right expression is nullptr)\n");
		tokens.rollbackTransaction();
		return nullptr;
	}

	// Create a function signature
	auto signature = language->createUnaryOperatorSignature(Type::Any, operatorToken.word, value->getType() );
	auto matchingOperator = language->findOperator(signature);

	if (matchingOperator != nullptr)
	{
		auto binOpNode = container->newUnaryOp(matchingOperator);

		// Connect the Left Operand :
		//---------------------------
		if (value->getOwner() == nullptr)
		{
            binOpNode->set("lvalue", value);
            delete value;
        }
		else
        {
			Node::Connect(value, binOpNode->get("lvalue"));
        }

		// Set the left !
        Member* result = binOpNode->get("result");

		LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " OK "\n");
        tokens.commitTransaction();

		return result;
	}
	else
	{
		LOG_VERBOSE("Parser", "parseUnaryOperationExpression... " KO " (unrecognysed operator)\n");
		tokens.rollbackTransaction();
		return nullptr;
	}
}

Member* Parser::parseAtomicExpression()
{
	LOG_VERBOSE("Parser", "parse atomic expr... \n");

	if ( !tokens.canEat() )
	{
		LOG_VERBOSE("Parser", "parse atomic expr... " KO "(not enough tokens)\n");
		return nullptr;
	}

	tokens.startTransaction();
	const Token& token = tokens.eatToken();
	if (token.type == TokenType::Operator)
	{
		LOG_VERBOSE("Parser", "parse atomic expr... " KO "(token is an operator)\n");
		tokens.rollbackTransaction();
		return nullptr;
	}

	auto result = tokenToMember(token);
	if( result != nullptr)
    {
	    tokens.commitTransaction();
        LOG_VERBOSE("Parser", "parse atomic expr... " OK "\n");
    }
	else
    {
        tokens.rollbackTransaction();
        LOG_VERBOSE("Parser", "parse atomic expr... " KO " (result is nullptr)\n");
	}

	return result;
}

Member* Parser::parseParenthesisExpression()
{
	LOG_VERBOSE("Parser", "parse parenthesis expr...\n");
	LOG_VERBOSE("Parser", "%s \n", tokens.toString().c_str());

	if ( !tokens.canEat() )
	{
		LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " no enough tokens.\n");
		return nullptr;
	}

	tokens.startTransaction();
	const Token& currentToken = tokens.eatToken();
	if (currentToken.type != TokenType::LBracket)
	{
		LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " open bracket not found.\n");
		tokens.rollbackTransaction();
		return nullptr;
	}

    Member* result = parseExpression();
	if (result)
	{
        const Token& token = tokens.eatToken();
		if ( token.type != TokenType::RBracket )
		{
			LOG_VERBOSE("Parser", "%s \n", tokens.toString().c_str());
			LOG_VERBOSE("Parser", "parse parenthesis expr..." KO " ( \")\" expected instead of %s )\n", token.word.c_str() );
            tokens.rollbackTransaction();
		}
		else
        {
			LOG_VERBOSE("Parser", "parse parenthesis expr..." OK  "\n");
            tokens.commitTransaction();
		}
	}
	else
    {
        LOG_VERBOSE("Parser", "parse parenthesis expr..." KO ", expression in parenthesis is nullptr.\n");
	    tokens.rollbackTransaction();
	}

	return result;
}

Member* Parser::parseRootExpression()
{
	Member* result = parseExpression();

	if ( tokens.canEat() )
	{
		LOG_VERBOSE("Parser", "parse root expression " KO " (not tokens not all consumed)\n");
	}

	if (result == nullptr)
	{
		LOG_VERBOSE("Parser", "parse root expression " KO " (result == nullptr)\n");
	}

	LOG_VERBOSE("Parser", "%s \n", tokens.toString().c_str());

	return result;
}

Member* Parser::parseExpression(unsigned short _precedence, Member* _leftOverride)
{
	LOG_VERBOSE("Parser", "parse expr...\n");
	LOG_VERBOSE("Parser", "%s \n", tokens.toString().c_str());

	if ( !tokens.canEat() )
	{
		LOG_VERBOSE("Parser", "parse expr..." KO " (unable to eat a single token)\n");
	}

	/*
		Get the left handed operand
	*/
	Member* left = nullptr;

	if (left = _leftOverride);
	else if (left = parseParenthesisExpression());
	else if (left = parseUnaryOperationExpression(_precedence));
	else if (left = parseFunctionCall());
	else if (left = parseAtomicExpression())

	if ( !tokens.canEat() )
	{
		LOG_VERBOSE("Parser", "parse expr... " OK " (last token reached)\n");
		return left;
	}

	Member* result;

	/*
		Get the right handed operand
	*/
	if ( left )
	{
		LOG_VERBOSE("Parser", "parse expr... left parsed, we parse right\n");
		auto binResult = parseBinaryOperationExpression(_precedence, left);

		if (binResult)
		{
			LOG_VERBOSE("Parser", "parse expr... right parsed, recursive call\n");
			result = parseExpression(_precedence, binResult);
		}
		else
        {
			result = left;
		}

	}
	else
    {
		LOG_VERBOSE("Parser", "parse expr... left is nullptr, we return it\n");
		result = left;
	}

	return result;
}



bool Parser::isSyntaxValid()
{
	bool success                     = true;
	auto it                          = tokens.getTokens().begin();
	short int openedParenthesisCount = 0;

	while( it != tokens.getTokens().end() && success == true) {

		auto current = *it;
		const bool isLastToken = tokens.getTokens().end() - it == 1;

		switch (current.type)
		{

		case TokenType::Operator:
		{

			if (isLastToken)
			{
				success = false; // Last token can't be an operator
			}
			else
            {
				auto next = *(it + 1);
				if (next.type == TokenType::Operator)
					success = false; // An operator can't be followed by another operator.
			}

			break;
		}
		case TokenType::LBracket:
		{
			openedParenthesisCount++;
			break;
		}
		case TokenType::RBracket:
		{
			openedParenthesisCount--;

			if (openedParenthesisCount < 0) {
				LOG_VERBOSE("Parser", "Unable to tokenize expression, mismatch parenthesis count. \n");
				success = false;
			}

			break;
		}
		default:
			break;
		}

		if (!isLastToken && current.isOperand()) { // Avoid an operand to be followed by another operand.
			auto next = *(it + 1);
			auto isAnOperand = next.isOperand();

			if (isAnOperand) {
				LOG_VERBOSE("Parser", "Unable to tokenize expression, %s unexpected after %s \n", current.word.c_str(), next.word.c_str());
				success = false;
			}
		}

		it++;
	}

	if (openedParenthesisCount != 0) // same opened/closed parenthesis count required.
		success = false;

	LOG_VERBOSE("Parser", "Parenthesis count = %i\n", openedParenthesisCount);

	return success;
}

bool Parser::tokenizeExpressionString(const std::string& _expression)
{

	/* get expression chars */
	auto chars = _expression;

	/* shortcuts to language members */
	auto regex    = language->getSemantic()->getTokenTypeToRegexMap();

	for(auto it = chars.cbegin(); it != chars.cend(); ++it)
	{
		std::string currStr    = chars.substr(it - chars.cbegin(), 1);
		auto        currDist   = std::distance(chars.cbegin(), it);

		// Unified parsing (loop over all regex)
		auto unifiedParsing = [&]() -> auto {
			for (auto pair_it = regex.cbegin(); pair_it != regex.cend(); pair_it++)
			{
				std::smatch sm;
				auto match = std::regex_search(it, chars.cend(), sm, pair_it->second);

				if (match) {
					auto str   = sm.str(0);
					auto token = pair_it->first;

					if (token != TokenType::Ignore)
					{
						if (token == TokenType::String)
						{
                            tokens.push(token, std::string(++str.cbegin(), --str.cend()),
                                        std::distance(chars.cbegin(), it));
						}
						else
                        {
                            tokens.push(token, str, std::distance(chars.cbegin(), it));
                        }
					}

					// advance iterator to the end of the str
					for (size_t i = 0; i < str.length() - 1; i++) it++;

					return true;
				}
			}
			return false;
		};

		if (!unifiedParsing())
		{
			return false;
		}
	}

	return true;

}

void TokenRibbon::push(TokenType  _type, const std::string& _string, size_t _charIndex)
{
	tokens.emplace_back(_type, _string, _charIndex);
}

TokenRibbon::TokenRibbon():
    currentTokenIndex(0)
{
    transactionStartTokenIndexes.push(0);
}

// TODO: highlight current transaction in another colo
std::string TokenRibbon::toString()const
{
    std::string result;

    for (auto it = tokens.begin(); it != tokens.end(); it++)
    {
        size_t index = it - tokens.begin();

        if ( index == currentTokenIndex )
        {
            result.append(GREEN);
            result.append((*it).word);
            result.append(RESET);
        }
        else
        {
            result.append((*it).word);
        }
    }

    const std::string endOfLine("<end>");

    if (tokens.size() == currentTokenIndex )
    {
        result.append(GREEN);
        result.append(endOfLine);
        result.append(RESET);
    }
    else
    {
        result.append(endOfLine);
    }

    return result;
}

const Token& TokenRibbon::eatToken()
{
    return tokens.at(currentTokenIndex++);
}

void TokenRibbon::startTransaction()
{
    transactionStartTokenIndexes.push(currentTokenIndex);
    LOG_VERBOSE("Parser", "Start Transaction (idx %i)\n", currentTokenIndex);
}

void TokenRibbon::rollbackTransaction()
{
    currentTokenIndex = transactionStartTokenIndexes.top();
    LOG_VERBOSE("Parser", "Rollback transaction (idx %i)\n", currentTokenIndex);
    transactionStartTokenIndexes.pop();
}

void TokenRibbon::commitTransaction()
{
    LOG_VERBOSE("Parser", "Commit transaction (idx %i)\n", currentTokenIndex);
    transactionStartTokenIndexes.pop();
}

void TokenRibbon::clear()
{
    tokens.clear();
    transactionStartTokenIndexes = std::stack<size_t>();
    currentTokenIndex = 0;
}

bool TokenRibbon::empty() const
{
    return tokens.empty();
}

size_t TokenRibbon::size() const
{
    return tokens.size();
}

bool TokenRibbon::canEat(size_t _tokenCount) const
{
    assert(_tokenCount > 0);
    return  currentTokenIndex + _tokenCount <= tokens.size() ;
}

const Token &TokenRibbon::peekToken() const
{
    return tokens.at(currentTokenIndex);
}

const std::vector<Token> &TokenRibbon::getTokens() const
{
    return tokens;
}

Member* Parser::parseFunctionCall()
{
	LOG_VERBOSE("Parser", "parse function call...\n");

	// Check if the minimum token count required is available ( 0: identifier, 1: open parenthesis, 2: close parenthesis)
	if ( !tokens.canEat(3) )
	{
		LOG_VERBOSE("Parser", "parse function call... " KO " aborted, not enough tokens.\n");
		return nullptr;
	}

    tokens.startTransaction();

    // Try to parse regular function: function(...)
    std::string identifier;
	const Token& token_0 = tokens.eatToken();
	const Token& token_1 = tokens.eatToken();
	if (token_0.type == TokenType::Symbol &&
		token_1.type == TokenType::LBracket)
	{
		identifier = token_0.word;
		LOG_VERBOSE("Parser", "parse function call... " OK " regular function pattern detected.\n");
	}
	else // Try to parse operator like (ex: operator==(..,..))
    {
        const Token &token_2 = tokens.eatToken(); // eat a "supposed open bracket"

        if (token_0.type == TokenType::Symbol && token_0.word == language->getSemantic()
                ->tokenTypeToString(TokenType::KeywordOperator /* TODO: TokenType::Keyword + word="operator" */) &&
            token_1.type == TokenType::Operator &&
            token_2.type == TokenType::LBracket)
        {
            // ex: "operator" + ">="
            identifier = token_0.word + token_1.word;
            LOG_VERBOSE("Parser", "parse function call... " OK " operator function-like pattern detected.\n");
        }
        else
        {
            LOG_VERBOSE("Parser", "parse function call... " KO " abort, this is not a function.\n");
            tokens.rollbackTransaction();
            return nullptr;
        }
    }
	std::vector<Member*> args;

	// Declare a new function prototype
	FunctionSignature signature(identifier, TokenType::AnyType);

	bool parsingError = false;
	while ( !parsingError && tokens.canEat() && tokens.peekToken().type != TokenType::RBracket)
	{

		if ( auto member = parseExpression() )
		{
			args.push_back(member); // store argument as member (already parsed)
			signature.pushArg( language->getSemantic()->typeToTokenType(member->getType()) );  // add a new argument type to the proto.

			if (tokens.peekToken().type == TokenType::Separator)
            {
			    tokens.eatToken();
            }
		}
		else
		{
			parsingError = true;
		}
	}

    // eat "close bracket supposed" token
	const Token& currToken = tokens.eatToken();
	if (currToken.type != TokenType::RBracket )
	{
		LOG_VERBOSE("Parser", "parse function call... " KO " abort, close parenthesis expected. \n");
		tokens.rollbackTransaction();
		return nullptr;
	}


	// Find the prototype in the language library
	auto fct = language->findFunction(signature);

	if( fct != nullptr)
	{
		auto node = container->newFunction(fct);

		auto connectArg = [&](size_t _argIndex)-> void { // lambda to connect input member to node for a specific argument index.

			auto arg = args.at(_argIndex);
			auto memberName = fct->signature.getArgs().at(_argIndex).name;

			if (arg->getOwner() == nullptr)
			{
				node->set(memberName.c_str(), arg);
			}
			else
            {
				Node::Connect(arg, node->get(memberName.c_str()));
			}
		};

		for( size_t argIndex = 0; argIndex < fct->signature.getArgs().size(); argIndex++ )
        {
            connectArg(argIndex);
        }

		tokens.commitTransaction();
		LOG_VERBOSE("Parser", "parse function call... " OK "\n");

		return node->get("result");

	}

	tokens.rollbackTransaction();
	LOG_VERBOSE("Parser", "parse function call... " KO " prototype not found.");
	return nullptr;
}
