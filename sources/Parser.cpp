#include "Parser.h"
#include "Log.h"          // for LOG_DEBUG(...)
#include "Member.h"
#include "Container.h"
#include "Variable.h"
#include "BinaryOperation.h"
#include "NodeView.h"
#include "Wire.h"
#include "Language.h"
#include "Log.h"

#include <algorithm>

#define DEBUG_PARSER // Enable detailed logs

#ifdef DEBUG_PARSER  // macro to disable these on debug
	#define LOG_DEBUG_PARSER(...) LOG_DEBUG(__VA_ARGS__)
#else
	#define LOG_DEBUG_PARSER(...)
#endif // DEBUG_PARSER

using namespace Nodable;

Parser::Parser(const Language* _language):language(_language)
{
	setMember("__class__", "Parser");
	addMember("expression", Visibility_VisibleOnlyWhenUncollapsed);
	setLabel("Parser");
}

Parser::~Parser()
{

}

std::string Parser::LogTokens(const std::vector<Token> _tokens, const size_t _highlight){
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

bool Parser::eval()
{
	bool success = false;

	if (!tokenizeExpressionString()) {
		LOG_DEBUG_PARSER("Unable to parse expression due to unrecognysed tokens.");
		return false;
	}

	if (!isSyntaxValid()) {
		LOG_DEBUG_PARSER("Unable to parse expression due to syntax error.");
		return false;
	}

	Member* resultValue = parseRootExpression();
	if (resultValue == nullptr) {
		LOG_DEBUG_PARSER("Unable to parse expression due to abstract syntax tree failure.");
		return false;
	}

	auto container   = this->getParent();
	Variable* result = container->createNodeResult();
	container->tryToRestoreResultNodePosition();

	// If the value has no owner, we simplly set the variable value
	if (resultValue->getOwner() == nullptr)
		result->setValue(resultValue);
	// Else we connect resultValue with resultVariable.value
	else
		Entity::Connect(container->createWire(), resultValue, result->getValueMember());


	NodeView::ArrangeRecursively(result->getComponent("view")->getAs<NodeView*>());
		
	success = true;

	return success;
}

Member* Parser::operandTokenToMember(const Token& _token) {


	Member* result = nullptr;

	switch (_token.type)
	{

		case TokenType_Boolean:
		{
			result = new Member();
			const bool value = _token.word == "true";
			result->setValue(value);
			break;
		}

		case TokenType_Symbol:
		{
			auto context = getParent();
			Variable* variable = context->find(_token.word);

			if (variable == nullptr)
				variable = context->createNodeVariable(_token.word);

			NODABLE_ASSERT(variable != nullptr);
			NODABLE_ASSERT(variable->getValueMember() != nullptr);

			result = variable->getValueMember();

			break;
		}

		case TokenType_Number: {
			result = new Member();
			const double number = std::stod(_token.word);
			result->setValue(number);
			break;
		}

		case TokenType_String: {
			result = new Member();
			result->setValue(_token.word);
			break;
		}

	}

	return result;
}

Member* Parser::parseBinaryOperationExpression(size_t& _tokenId, unsigned short _precedence, Member* _left, unsigned short _depth) {

	LOG_DEBUG_PARSER("parseBinaryOperationExpression...\n");
	LOG_DEBUG_PARSER("%s \n", Parser::LogTokens(tokens, _tokenId).c_str());

	Member* result = nullptr;

	if (_tokenId + 1 >= tokens.size()) {
		LOG_DEBUG_PARSER("parseBinaryOperationExpression... " KO " (not enought tokens)\n");
		return nullptr;
	}

	const Token& token1(tokens.at(_tokenId));
	const Token& token2(tokens.at(_tokenId+1));

	// Structure check
	const bool isValid = _left != nullptr &&
			             token1.type == TokenType_Operator &&
			             token2.type != TokenType_Operator;

	if (!isValid) {
		LOG_DEBUG_PARSER("parseBinaryOperationExpression... " KO " (Structure)\n");
		return nullptr;
	}

	// Precedence check
	const auto currentOperatorPrecedence = language->getOperatorPrecedence(token1.word);
		
	if (currentOperatorPrecedence <= _precedence) { // always eval the first operation if they have the same precedence or less.
		LOG_DEBUG_PARSER("parseBinaryOperationExpression... " KO " (Precedence)\n");
		return nullptr;
	}
		

	// Parse right expression
	size_t rightTokenId = _tokenId + 1;
	auto right = parseExpression(rightTokenId, currentOperatorPrecedence, nullptr, _depth +1 );

	if (!right) {
		LOG_DEBUG_PARSER("parseBinaryOperationExpression... " KO " (right expression is nullptr)\n");
		return nullptr;
	}

	// Build the graph for the first 3 tokens
	Container* context = this->getParent();	

	// Special behavior for "=" operator
	if (token1.word == "=") {

		// left operand (should BE a variable)

		NODABLE_ASSERT(_left->getOwner() != nullptr); // left operand cannot be a orphaned member
		NODABLE_ASSERT(_left->getOwner()->getMember("__class__")->getValueAsString() == "Variable"); // left operand need to me owned by a variable node			               


		// Directly connects right operand output to left operant input (yes that's reversed compared to code)
		if (right->getOwner() == nullptr)
			_left->getOwner()->setMember("value", right);
		else
			Entity::Connect(context->createWire(), right, _left);

		result = _left->getOwner()->getFirstMemberWithConnection(Connection_InOut);


	// For all other binary operations :
	} else {
		auto binOperation = context->createNodeBinaryOperation(token1.word);

		// Connect the Left Operand :
		//---------------------------
		if (_left->getOwner() == nullptr)
			binOperation->setMember("left", _left);
		else
			Entity::Connect(context->createWire(), _left, binOperation->getMember("left"));

		// Connect the Right Operand :

		if (right->getOwner() == nullptr)
			binOperation->setMember("right", right);
		else
			Entity::Connect(context->createWire(), right, binOperation->getMember("right"));

		// Set the left !
		result = binOperation->getMember("result");
	}

	_tokenId = rightTokenId;	

	LOG_DEBUG_PARSER("parseBinaryOperationExpression... " OK "\n");

	return result;
}

Member* Parser::parseUnaryOperationExpression(size_t& _tokenId, unsigned short _precedence, unsigned short _depth) {

	LOG_DEBUG_PARSER("parseUnaryOperationExpression...\n");
	LOG_DEBUG_PARSER("%s \n", Parser::LogTokens(tokens, _tokenId).c_str());

	auto logPrefix = ComputePrefix(_depth);

	Member* result = nullptr;

	const bool hasEnoughtTokens = tokens.size() > _tokenId + 1;
	if (!hasEnoughtTokens)
		return nullptr;

	const Token& token1(tokens.at(_tokenId));
	const Token& token2(tokens.at(_tokenId+1));

	// Check if we get an operator first
	if (token1.type != TokenType_Operator) {
		LOG_DEBUG_PARSER("parseUnaryOperationExpression... " KO " (operator not found)\n");
		return nullptr;
	}

	// Then check if the operator can be applied to the next token
	if (token1.word == "-" && token2.type == TokenType_Number) { // TODO: create the unary operation "negates"
		result = operandTokenToMember(token2);
		result->setValue(-result->getValueAsNumber());

	} else if (token1.word == "!" && token2.type == TokenType_Boolean) { // TODO: create the unary operation "not"
		result = operandTokenToMember(token2);
		result->setValue(!result->getValueAsBoolean());

	} else {
		LOG_DEBUG_PARSER("parseUnaryOperationExpression... " KO " (unrecognysed operator)\n");	
		return nullptr;
	}

	_tokenId += 2;
	LOG_DEBUG_PARSER("parseUnaryOperationExpression... " OK "\n");

	return result;
}

Member* Parser::parseAtomicExpression(size_t& _tokenId, unsigned short _depth) {

	LOG_DEBUG_PARSER("parseAtomicExpression... \n");

	// Check if there is index is not out of bounds
	if (tokens.size() <= _tokenId) {
		LOG_DEBUG_PARSER("parseAtomicExpression... " KO "(not enought tokens)\n");
		return nullptr;
	}

	auto token = tokens.at(_tokenId);

	// Check if token is not an operator
	if (token.type == TokenType_Operator) {
		LOG_DEBUG_PARSER("parseAtomicExpression... " KO "(token is an operator)\n");
		return nullptr;
	}

	_tokenId++;

	LOG_DEBUG_PARSER("parseAtomicExpression... " OK "\n");

	return operandTokenToMember(token);
}

Member* Parser::parseParenthesisExpression(size_t& _tokenId, unsigned short _depth) {

	LOG_DEBUG("parseParenthesisExpression...");
	LOG_DEBUG("%s \n", Parser::LogTokens(tokens, _tokenId).c_str());

	auto logPrefix = ComputePrefix(_depth);

	if (_tokenId >= tokens.size())
		return nullptr;

	auto token1(tokens.at(_tokenId));

	if (token1.type != TokenType_Parenthesis) {
		return nullptr;
	}

	Member* result(nullptr);

	if (token1.word == "(") {

		auto subToken = _tokenId + 1;
		result = parseExpression(subToken, 0u, nullptr, _depth + 1);
		_tokenId = subToken + 1;

		if ( tokens.at(subToken).word != ")" ) {
			LOG_DEBUG("%s \n", Parser::LogTokens(tokens, _tokenId).c_str());
			LOG_DEBUG_PARSER("parseParenthesisExpression failed... " KO " ( \")\" expected after %s )\n", tokens.at(subToken - 1) );
		} else {
			LOG_DEBUG_PARSER("parseParenthesisExpression... " OK  "\n");
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

	LOG_DEBUG_PARSER("%s \n", Parser::LogTokens(tokens, tokenId).c_str());

	return result;
}

Member* Parser::parseExpression(size_t& _tokenId, unsigned short _precedence, Member* _leftOverride, unsigned short _depth) {

	LOG_DEBUG_PARSER("parseExpression...\n");
	LOG_DEBUG_PARSER("%s \n", Parser::LogTokens(tokens, _tokenId).c_str());

	if (_tokenId >= tokens.size()) {
		LOG_DEBUG_PARSER("parseExpression..." KO " (last token)\n");
	}

	/**
		Get the left handed operand
	*/
	Member* left = nullptr;

	if      (left = _leftOverride) {}
	else if (left = parseParenthesisExpression(_tokenId, _depth + 1)) {}
	else if (left = parseUnaryOperationExpression(_tokenId, _precedence, _depth + 1)) {}
	else if (left = parseAtomicExpression(_tokenId, _depth + 1)) {}	

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
		auto binResult = parseBinaryOperationExpression(_tokenId, _precedence, left, _depth + 1);

		if (binResult) {
			LOG_DEBUG_PARSER("right parsed, recursive call\n");
			result = parseExpression(_tokenId, _precedence, binResult, _depth + 1);
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

		case TokenType_Operator:
		{
			
			if (isLastToken) { 
				success = false; // Last token can't be an operator

			} else {
				auto next = *(it + 1);
				if (next.type == TokenType_Operator)
					success = false; // An operator can't be followed by another operator.
			}

			break;
		}
		case TokenType_Parenthesis:
		{
			const bool isOpenParenthesis = current.word == "(";
			openedParenthesisCount += isOpenParenthesis ? 1 : -1; // increase / decrease openend parenthesis count.

			if (openedParenthesisCount < 0) {
				LOG_WARNING("Unable to tokenize expression, mismatch parenthesis count. \n");
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

bool Parser::tokenizeExpressionString()
{

	/* get expression chars */
	std::string chars = getMember("expression")->getValueAsString();

	/* prepare allowed chars */
	const auto numbers 	     = language->numbers;
	const auto letters		 = language->letters;
	const auto operators 	 = language->getOperatorsAsString();

	/* prepare reserved keywords */
	const auto keywords = language->keywords;

	for(auto it = chars.begin(); it != chars.end(); ++it)
	{
		//---------------
		// Term -> Comment
		//---------------
		if ( chars.end() - it >= 2 && std::string(it, it +2) == "//") {
			it = chars.end() -1  ;

		//---------------
		// Term -> Number
		//---------------
		} else if( numbers.find(*it) != std::string::npos ) {

			auto itStart = it;
			while(	it != chars.end() && 
					numbers.find(*it) != std::string::npos)
			{
				++it;
			}
						
			--it;

			std::string number = chars.substr(itStart - chars.begin(), it - itStart + 1);
			addToken(TokenType_Number, number, std::distance(chars.begin(), itStart) );
			
		//----------------
		// Term -> String
		//----------------

		}else 	if(*it == '"')
		{
			++it;

			if (it != chars.end())
			{
				auto itStart = it;
				while (it != chars.end() && *it != '"')
				{
					++it;
				}

				if (it != chars.end() && *it == '"')
				{
					std::string str = chars.substr(itStart - chars.begin(), it - itStart);
					addToken(TokenType_String, str, std::distance(chars.begin(), itStart));
					--it;
				}else {
					--it;
				}

			}
			else {
				--it;
			}

		//----------------------------
		// Term -> { Symbol, Keyword }
		//----------------------------

		}else 	if( letters.find(*it) != std::string::npos)
		{
			auto itStart = it;
			while(	it != chars.end() && 
					letters.find(*it) != std::string::npos)
			{
				++it;
			}
			--it;

			std::string str = chars.substr(itStart - chars.begin(), it - itStart + 1);

			//-----------------
			// Term -> Keyword
			//-----------------
			if ( keywords.find(str) != keywords.end())
				addToken(keywords.at(str), str, std::distance(chars.begin(), itStart));

			//-----------------
			// Term -> Symbol
			//-----------------
			else
				addToken(TokenType_Symbol, str, std::distance(chars.begin(), itStart));

		//-----------------
		// Term -> Operator
		//-----------------
			
		}else 	if(operators.find(*it) != std::string::npos)
		{
			std::string str = chars.substr(it - chars.begin(), 1);
			addToken(TokenType_Operator, str, std::distance(chars.begin(), it));

		//-----------------
		// Term -> Parenthesis
		//-----------------
			
		}else 	if(*it == ')' || *it == '(' )
		{
			std::string str = chars.substr(it - chars.begin(), 1);
			addToken(TokenType_Parenthesis, str, std::distance(chars.begin(), it));

		}else if ( *it != ' ') {
			LOG_WARNING("Unable to tokenize expression %s \n", chars);
			return false;
		}
	}

	return true;

}

void Parser::addToken(TokenType_  _type, std::string _string, size_t _charIndex)
{
	Token t;
	t.type      = _type;
	t.word      = _string;
	t.charIndex = _charIndex;

	tokens.push_back(t);
}
