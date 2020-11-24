#include "Parser.h"
#include "Log.h"          // for LOG_DEBUG(...)
#include "Member.h"
#include "Container.h"
#include "Variable.h"
#include "Wire.h"
#include "Language.h"
#include "Log.h"
#include <regex>
#include <algorithm>

// Enable detailed logs
// #define DEBUG_PARSER

#ifdef DEBUG_PARSER  // macro to disable these on debug
	#define LOG_DEBUG_PARSER(...) LOG_DEBUG(__VA_ARGS__)
#else
	#define LOG_DEBUG_PARSER(...)
#endif // DEBUG_PARSER

using namespace Nodable;

Parser::Parser(const Language* _language, Container* _container):
	                 language(_language), container(_container)
{
}

Parser::~Parser()
{

}

std::string Parser::logTokens(const std::vector<Token> _tokens, const size_t _highlight){
	std::string result;

	for (auto it = _tokens.begin(); it != _tokens.end(); it++ ) {
		size_t index = it - _tokens.begin();

		if (index == _highlight) {
			result.append(GREEN);
			result.append((*it).word);
			result.append(RESET);
		} else {
			result.append((*it).word);
		}
	}

	const std::string endOfLine(" (last)");

	if (_tokens.size() == _highlight) {
		result.append(GREEN);
		result.append(endOfLine);
		result.append(RESET);
	} else {
		result.append(endOfLine);
	}

	return result;
}

bool Parser::eval(const std::string& _expression)
{

	if (!tokenizeExpressionString(_expression)) {
		LOG_WARNING(Log::Verbosity::Normal, "Unable to parse expression due to unrecognysed tokens.\n");
		return false;
	}

	if(tokens.size() == 0 )
    {
        LOG_MESSAGE(Log::Verbosity::Normal, "Nothing to evaluate.\n");
        return false;
    }

	if (!isSyntaxValid()) {
		LOG_WARNING(Log::Verbosity::Normal, "Unable to parse expression due to syntax error.\n");
		return false;
	}

	Member* resultValue = parseRootExpression();
	if (resultValue == nullptr) {
		LOG_WARNING(Log::Verbosity::Normal, "Unable to parse expression due to abstract syntax tree failure.\n");
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
        Node::Connect(resultValue, result->getMember());
    }

	LOG_MESSAGE(Log::Verbosity::Normal, "Expression evaluated: %s\n", _expression.c_str() );
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
			NODABLE_ASSERT(variable->getMember() != nullptr);

			result = variable->getMember();

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

	}

	return result;
}

Member* Parser::parseBinaryOperationExpression(size_t& _tokenId, unsigned short _precedence, Member* _left) {

	LOG_DEBUG_PARSER("parseBinaryOperationExpression...\n");
	LOG_DEBUG_PARSER("%s \n", Parser::logTokens(tokens, _tokenId).c_str());

	Member* result = nullptr;

	if (_tokenId + 1 >= tokens.size()) {
		LOG_DEBUG_PARSER("parseBinaryOperationExpression... " KO " (not enought tokens)\n");
		return nullptr;
	}

	const Token& token1(tokens.at(_tokenId));
	const Token& token2(tokens.at(_tokenId+1));

	// Structure check
	const bool isValid = _left != nullptr &&
			             token1.type == TokenType::Operator &&
			             token2.type != TokenType::Operator;

	if (!isValid) {
		LOG_DEBUG_PARSER("parseBinaryOperationExpression... " KO " (Structure)\n");
		return nullptr;
	}

	// Precedence check
	const auto currentOperatorPrecedence = language->findOperator(token1.word)->precedence;

	if (currentOperatorPrecedence <= _precedence &&
	    _precedence > 0u) { // always eval the first operation if they have the same precedence or less.
		LOG_DEBUG_PARSER("parseBinaryOperationExpression... " KO " (Precedence)\n");
		return nullptr;
	}


	// Parse right expression
	size_t rightTokenId = _tokenId + 1;
	auto right = parseExpression(rightTokenId, currentOperatorPrecedence, nullptr );

	if (!right) {
		LOG_DEBUG_PARSER("parseBinaryOperationExpression... " KO " (right expression is nullptr)\n");
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

	}else {
		LOG_DEBUG_PARSER("parseBinaryOperationExpression... " KO " (unable to find operator prototype)\n");
		return nullptr;
	}

	_tokenId = rightTokenId;

	LOG_DEBUG_PARSER("parseBinaryOperationExpression... " OK "\n");

	return result;
}

Member* Parser::parseUnaryOperationExpression(size_t& _tokenId, unsigned short _precedence) {

	LOG_DEBUG_PARSER("parseUnaryOperationExpression...\n");
	LOG_DEBUG_PARSER("%s \n", Parser::logTokens(tokens, _tokenId).c_str());

	Member* result = nullptr;

	const bool hasEnoughtTokens = tokens.size() > _tokenId + 1;
	if (!hasEnoughtTokens)
		return nullptr;

	const Token& token1(tokens.at(_tokenId));

	// Check if we get an operator first
	if (token1.type != TokenType::Operator) {
		LOG_DEBUG_PARSER("parseUnaryOperationExpression... " KO " (operator not found)\n");
		return nullptr;
	}

	// Parse expression after the operator
	auto valueTokenId = _tokenId + 1;
	auto precedence = language->findOperator(token1.word)->precedence;
	Member* value = nullptr;

	     if ( value = parseAtomicExpression(valueTokenId));
	else if ( value = parseParenthesisExpression(valueTokenId));
	else
	{
		LOG_DEBUG_PARSER("parseUnaryOperationExpression... " KO " (right expression is nullptr)\n");
		return nullptr;
	}

	// Create a function signature
	auto signature = language->createUnaryOperatorSignature(Type::Any, token1.word, value->getType() );
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
		result = binOpNode->get("result");

	} else {
		LOG_DEBUG_PARSER("parseUnaryOperationExpression... " KO " (unrecognysed operator)\n");
		return nullptr;
	}

	_tokenId = valueTokenId;
	LOG_DEBUG_PARSER("parseUnaryOperationExpression... " OK "\n");

	return result;
}

Member* Parser::parseAtomicExpression(size_t& _tokenId) {

	LOG_DEBUG_PARSER("parseAtomicExpression... \n");

	// Check if there is index is not out of bounds
	if (tokens.size() <= _tokenId) {
		LOG_DEBUG_PARSER("parseAtomicExpression... " KO "(not enought tokens)\n");
		return nullptr;
	}

	auto token = tokens.at(_tokenId);

	// Check if token is not an operator
	if (token.type == TokenType::Operator) {
		LOG_DEBUG_PARSER("parseAtomicExpression... " KO "(token is an operator)\n");
		return nullptr;
	}

	auto result = tokenToMember(token);

	if( result != nullptr)
		_tokenId++;

	LOG_DEBUG_PARSER("parseAtomicExpression... " OK "\n");

	return result;
}

Member* Parser::parseParenthesisExpression(size_t& _tokenId) {

	LOG_DEBUG_PARSER("parseParenthesisExpression...");
	LOG_DEBUG_PARSER("%s \n", Parser::logTokens(tokens, _tokenId).c_str());

	if (_tokenId >= tokens.size())
		return nullptr;

	auto token1(tokens.at(_tokenId));

	if (token1.type != TokenType::LBracket) {
		return nullptr;
	}

	Member* result(nullptr);

	if (token1.word == "(") {

		auto subToken = _tokenId + 1;
		result = parseExpression(subToken, 0u, nullptr);

		if (result)
		{
			_tokenId = subToken + 1;

			if (tokens.at(subToken).word != ")") {
				LOG_DEBUG_PARSER("%s \n", Parser::logTokens(tokens, _tokenId).c_str());
				LOG_DEBUG_PARSER("parseParenthesisExpression failed... " KO " ( \")\" expected after %s )\n", tokens.at(subToken - 1));
			}
			else {
				LOG_DEBUG_PARSER("parseParenthesisExpression... " OK  "\n");
			}
		}
	} else {
		LOG_DEBUG_PARSER("parseParenthesisExpression... " KO " (open parenthesis not found) \n");
	}


	return result;
}

Member* Parser::parseRootExpression() {

	size_t  tokenId      = 0;
	Member* result       = nullptr;
	bool    parsingError = false;

	result = parseExpression(tokenId, 0u, result);

	const auto tokenLeft = tokens.size() - tokenId;
	if (tokenLeft != 0) {   // Check if all tokens have been consumed
		LOG_DEBUG_PARSER("parse root expression " KO " (not tokens not all consumed)");
	}

	if (result == nullptr) { // Check if result is defined
		LOG_DEBUG_PARSER("parse root expression " KO " (result == nullptr)\n");
	}

	LOG_DEBUG_PARSER("%s \n", Parser::logTokens(tokens, tokenId).c_str());

	return result;
}

Member* Parser::parseExpression(size_t& _tokenId, unsigned short _precedence, Member* _leftOverride) {

	LOG_DEBUG_PARSER("parseExpression...\n");
	LOG_DEBUG_PARSER("%s \n", Parser::logTokens(tokens, _tokenId).c_str());

	if (_tokenId >= tokens.size()) {
		LOG_DEBUG_PARSER("parseExpression..." KO " (last token)\n");
	}

	/**
		Get the left handed operand
	*/
	Member* left = nullptr;

	if (left = _leftOverride);
	else if (left = parseParenthesisExpression(_tokenId));
	else if (left = parseUnaryOperationExpression(_tokenId, _precedence));
	else if (left = parseFunctionCall(_tokenId));
	else if (left = parseAtomicExpression(_tokenId))

	if (_tokenId >= tokens.size()) {
		LOG_DEBUG_PARSER("parseExpression..." OK " (parsing only left, last token)\n");
		return left;
	}

	Member* result;

	/**
		Get the right handed operand
	*/

	if (left != nullptr) {

		LOG_DEBUG_PARSER("left parsed, we parse right\n");
		auto binResult = parseBinaryOperationExpression(_tokenId, _precedence, left);

		if (binResult) {
			LOG_DEBUG_PARSER("right parsed, recursive call\n");
			result = parseExpression(_tokenId, _precedence, binResult);
		}
		else {
			result = left;
		}

	} else {
		LOG_DEBUG_PARSER("left is nullptr, we return it\n");
		result = left;
	}

	return result;
}



bool Parser::isSyntaxValid()
{
	bool success                     = true;
	auto it                          = tokens.begin();
	short int openedParenthesisCount = 0;

	while( it != tokens.end() && success == true) {

		auto current = *it;
		const bool isLastToken = tokens.end() - it == 1;

		switch (current.type)
		{

		case TokenType::Operator:
		{

			if (isLastToken) {
				success = false; // Last token can't be an operator

			} else {
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
				LOG_DEBUG_PARSER("Unable to tokenize expression, mismatch parenthesis count. \n");
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
				LOG_DEBUG_PARSER("Unable to tokenize expression, %s unexpected after %s \n", current.word.c_str(), next.word.c_str());
				success = false;
			}
		}

		it++;
	}

	if (openedParenthesisCount != 0) // same opened/closed parenthesis count required.
		success = false;

	LOG_DEBUG_PARSER("Parenthesis count = %i\n", openedParenthesisCount);

	return success;
}

bool Parser::tokenizeExpressionString(const std::string& _expression)
{

	/* get expression chars */
	auto chars = _expression;

	/* shortcuts to language members */
	auto regex    = language->dictionnary.getTokenTypeToRegexMap();

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
							addToken(token, std::string(++str.cbegin(), --str.cend()), std::distance(chars.cbegin(), it));
						else
							addToken(token, str, std::distance(chars.cbegin(), it));
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

void Parser::addToken(TokenType  _type, std::string _string, size_t _charIndex)
{
	Token t;
	t.type      = _type;
	t.word      = _string;
	t.charIndex = _charIndex;

	tokens.push_back(t);
}

Member* Parser::parseFunctionCall(size_t& _tokenId)
{
	size_t localTokenId = _tokenId;

	std::string identifier;

	// Check if the minimum token count required is available ( 0: identifier, 1: open parenthesis, 2: close parenthesis)
	if (localTokenId + 2 >= tokens.size() )
	{
		LOG_DEBUG_PARSER("parseFunctionCall aborted. Not enough tokens.");
		return nullptr;
	}


	const auto token_0 = tokens.at(localTokenId);
	const auto token_1 = tokens.at(localTokenId + 1);
	const auto token_2 = tokens.at(localTokenId + 2);

	// regular function
	if (token_0.type == TokenType::Symbol &&
		token_1.type == TokenType::LBracket)
	{
		identifier = token_0.word;
		localTokenId++;
	}

	// operator like (ex: operator==(..,..))
	else if (token_0.type == TokenType::Symbol &&
		     token_1.type == TokenType::Operator &&
		     token_2.type == TokenType::LBracket)
	{
                // ex: "operator" + ">="
		identifier = token_0.word + token_1.word;
		localTokenId += 2;
	}
	else
	{
		LOG_DEBUG_PARSER("parseFunctionCall aborted. (no regular function or function-like operator found)");
		return nullptr;
	}

	std::vector<Member*> args;

	// Declare a new function prototype
	FunctionSignature signature(identifier, TokenType::AnyType);

	localTokenId++; // eat parenthesis

	bool parsingError = false;
	while ( !parsingError &&
		     localTokenId < tokens.size() &&
		     tokens.at(localTokenId).type != TokenType::RBracket)
	{

		if (auto member = parseExpression(localTokenId))
		{
			args.push_back(member); // store argument as member (already parsed)
			signature.pushArg( language->typeToTokenType(member->getType()) );  // add a new argument type to the proto.

			if (tokens.at(localTokenId).type == TokenType::Separator)
				localTokenId++;

		}
		else
		{
			parsingError = true;
		}
	}


	if (tokens.at(localTokenId).type != TokenType::RBracket ) {
		LOG_DEBUG_PARSER("parseFunctionCall aborted. Close parenthesis expected !");
		return nullptr;
	}

	localTokenId++; // eat parenthesis

	// Find the prototype in the language library
	auto fct = language->findFunction(signature);

	if( fct != nullptr) { // if function found

		auto node = container->newFunction(fct);

		auto connectArg = [&](size_t _argIndex)-> void { // lambda to connect input member to node for a specific argument index.

			auto arg = args.at(_argIndex);
			auto memberName = fct->signature.getArgs().at(_argIndex).name;

			if (arg->getOwner() == nullptr) {
				node->set(memberName.c_str(), arg);
			} else {
				Node::Connect(arg, node->get(memberName.c_str()));
			}
		};

		for( size_t argIndex = 0; argIndex < fct->signature.getArgs().size(); argIndex++ )
			connectArg(argIndex);

		_tokenId = localTokenId;
		return node->get("result");

	} else {
		LOG_DEBUG_PARSER("Unable to parse function, prototype not found.");
	}

	return nullptr;
}
