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

bool Parser::eval()
{
	bool success = false;

	if (!tokenizeExpressionString()) {
		LOG_DEBUG("Unable to parse expression due to unrecognysed tokens.");
		return false;
	}

	if (!isSyntaxValid()) {
		LOG_DEBUG("Unable to parse expression due to syntax error.");
		return false;
	}

	Member* resultValue = parseRootExpression();
	if (resultValue == nullptr) {
		LOG_DEBUG("Unable to parse expression due to abstract syntax tree failure.");
		return false;
	}

	auto container   = this->getParent();
	Variable* result = container->createNodeResult();

	// If the value has no owner, we simplly set the variable value
	if (resultValue->getOwner() == nullptr)
		result->setValue(resultValue);
	// Else we connect resultValue with resultVariable.value
	else
		Entity::Connect(container->createWire(), resultValue, result->getValueMember());


	// Hides the value member only if it is connected to something (to reduce screen space used)
	auto member = result->getMember("value");
	if ( member->getInputMember() != nullptr)
		member->setVisibility(Visibility_VisibleOnlyWhenUncollapsed);

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

Member* Parser::buildGraphIterative()
{
	Member*    result  = nullptr;
	Container* context = this->getParent();

	NODABLE_ASSERT(context != nullptr);

	// Computes the number of token to eval
	size_t tokenCount = tokens.size();
	size_t cursor = 0;

	Member* _leftOverride = nullptr;
	Member* _rightOverride = nullptr;
	
	Entity* previousBinaryOperation = nullptr;

	while (cursor < tokenCount && result == nullptr) {

		size_t tokenLeft = tokenCount - cursor;
		Member* tempResult = nullptr;

		switch (tokenLeft) {

			// Operand
			case 1: {
				const Token& token(tokens[cursor]);
				tempResult = operandTokenToMember(token);
				cursor += 1;
				break;
			}

			// Operator, Operand
			case 2: {
				const Token& token1(tokens.at(cursor));
				const Token& token2(tokens.at(cursor + 1));

				if (token1.type == TokenType_Operator) {

					if (token1.word == "-" && token2.type == TokenType_Number) {
						tempResult = operandTokenToMember(token2);
						tempResult->setValue(-tempResult->getValueAsNumber());
					}
					else if (token1.word == "!" && token2.type == TokenType_Boolean) {
						tempResult = operandTokenToMember(token2);
						tempResult->setValue(!tempResult->getValueAsBoolean());
					}
				}
				cursor += 2;
				break;
			}

			// Operand, Operator, Expression
			default: {
				const Token& token1(tokens.at(cursor));
				const Token& token2(tokens.at(cursor + 1));
				const Token& token3(tokens.at(cursor + 2));
				
				/* Check if we are in Operand, Operator, Expression state*/
				if (token1.type == TokenType_Operator ||
					token2.type != TokenType_Operator ||
					token3.type == TokenType_Operator)
					return result;

				// Generate operation and members
				auto left      = _leftOverride ? _leftOverride : operandTokenToMember(token1);
				auto operator1 = context->createNodeBinaryOperation(token2.word);
				auto right     = _rightOverride ? _rightOverride : operandTokenToMember(token3);

				// If we get a more that 3 terms expression, we need to compute operator precedence
				if ( tokenLeft > 3 ) {

					const Token& token4(tokens.at(cursor + 3));

					bool firstOperatorHasHigherPrecedence = BinaryOperationComponent::NeedsToBeEvaluatedFirst(token2.word, token4.word);

					if (!firstOperatorHasHigherPrecedence) {
						// Evaluate the rest of the expression
						// auto _rightOverride = buildGraphIterative(cursor + 2, 0);
					}

				}

				// Connect the Left
				if (left->getOwner() == nullptr)
					operator1->setMember("left", left);
				else
					Entity::Connect(context->createWire(), left, operator1->getMember("left"));

				// Connect the Right
				if (right->getOwner() == nullptr)
					operator1->setMember("right", right);
				else
					Entity::Connect(context->createWire(), right, operator1->getMember("right"));

				// Set the left
				tempResult = operator1->getMember("result");

				// For now force execution of left operator before right
				_rightOverride = nullptr;
				_leftOverride = tempResult;
				previousBinaryOperation = operator1;
				cursor += 2;

				break;
			}
		}

		if (cursor >= tokenCount)
			result = tempResult;
	}

	return result;
}

Member* Parser::parseBinaryOperationExpression(size_t& _tokenId, unsigned short _precedence, Member* _left) {

	Member* result = nullptr;

	if (_tokenId + 1 >= tokens.size())
		return nullptr;

	const Token& token1(tokens.at(_tokenId));
	const Token& token2(tokens.at(_tokenId+1));

	// Structure check
	const bool isValid = _left != nullptr &&
			             token1.type == TokenType_Operator &&
			             token2.type != TokenType_Operator;

	if (!isValid) {
		return nullptr;
	}
		 
	// Precedence check
	const auto currentOperatorPrecedence = language->getOperatorPrecedence(token1.word);
		
	if (currentOperatorPrecedence < _precedence)
		return nullptr;

	LOG_DEBUG("parseBinaryOperationExpression... _tokenId=%lu, _precedence=%u \n", _tokenId, _precedence);

	// Parse right expression
	size_t rightTokenId = _tokenId + 1;
	auto right = parseExpression(rightTokenId, currentOperatorPrecedence);

	if (!right)
		return nullptr;

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

	return result;
}

Member* Parser::parseUnaryOperationExpression(size_t& _tokenId, unsigned short _precedence) {

	Member* result = nullptr;

	const bool hasEnoughtTokens = tokens.size() > _tokenId + 1;
	if (!hasEnoughtTokens)
		return result;

	const Token& token1(tokens.at(_tokenId++));
	const Token& token2(tokens.at(_tokenId++));

	if (token1.type != TokenType_Operator) {
		_tokenId -= 2;
		return result;
	}

	LOG_DEBUG("parseUnaryOperationExpression...\n");

	// TODO: create the unary operation "negates"
	if (token1.word == "-" && token2.type == TokenType_Number) {
		result = operandTokenToMember(token2);
		result->setValue(-result->getValueAsNumber());
	}

	// TODO: create the unary operation "not"
	else if (token1.word == "!" && token2.type == TokenType_Boolean) {
		result = operandTokenToMember(token2);
		result->setValue(!result->getValueAsBoolean());
	}

	return result;
}

Member* Parser::parsePrimaryExpression(size_t& _tokenId) {

	// Check if there is index is not out of bounds
	if (tokens.size() <= _tokenId)
		return nullptr;

	auto token = tokens.at(_tokenId++);

	// Check if token is not an operator
	if (token.type == TokenType_Operator) {
		_tokenId--;
		return nullptr;
	}

	LOG_DEBUG("parsePrimaryExpression... _tokenId=%lu \n", _tokenId);

	return operandTokenToMember(token);
}

Member* Parser::parseSubExpression(size_t& _tokenId) {


	if (_tokenId >= tokens.size())
		return nullptr;

	auto token1(tokens.at(_tokenId));

	if (token1.type != TokenType_Parenthesis) {
		return nullptr;
	}

	Member* result(nullptr);

	LOG_DEBUG("parseSubExpression... _tokenId=%lu \n", _tokenId);

	if (token1.word == "(") {
		LOG_DEBUG("parseSubExpression...\n");
		auto subToken = _tokenId + 1;
		result = parseExpression(subToken, 0u);

		if (tokens.size() <= subToken || tokens.at(subToken).word != ")") {
			LOG_DEBUG(" ) expected after ", tokens.at(subToken - 1));
		}

		_tokenId = subToken + 1;
	}


	return result;
}

Member* Parser::parseRootExpression() {

	size_t         tokenId(0);
	Member* result(nullptr);
	bool           parsingError(false);

	while (tokenId < tokens.size()) {
		auto intermediateResult = parseExpression(tokenId, 0u, result);
		result = intermediateResult;
	}

	return result;
}

Member* Parser::parseExpression(size_t& _tokenId, unsigned short _precedence, Member* _leftOverride) {

	LOG_DEBUG("parseExpression... _tokenId=%lu, _precedence=%u \n", _tokenId, _precedence);

	Member* left = nullptr;

	if (left = _leftOverride) {

	}
	else if (left = parseSubExpression(_tokenId)) {
		LOG_DEBUG("sub expression expression parsed... _tokenId=%lu, _precedence=%u \n", _tokenId, _precedence);

	}
	else if (left = parseUnaryOperationExpression(_tokenId, _precedence)) {
		LOG_DEBUG("unary expression parsed... _tokenId=%lu, _precedence=%u \n", _tokenId, _precedence);

	}
	else if (left = parsePrimaryExpression(_tokenId)) {

	}



	if (left != nullptr) {

		auto binResult = parseBinaryOperationExpression(_tokenId, _precedence, left);

		if (binResult) {
			return binResult;
		}

	}

	return left;
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
				LOG_WARNING("Unable to tokenize expression, %s unexpected after %s \n", current.word.c_str(), next.word.c_str());
				success = false;
			}
		}

		it++;
	}

	if (openedParenthesisCount != 0) // same opened/closed parenthesis count required.
		success = false;

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
