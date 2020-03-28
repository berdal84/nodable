#include "Parser.h"
#include "Log.h"          // for LOG_DBG(...)
#include "Member.h"
#include "Container.h"
#include "Variable.h"
#include "BinaryOperation.h"
#include "NodeView.h"
#include "Wire.h"
#include "Language.h"
#include <algorithm>

using namespace Nodable;

Parser::Parser(const Language* _language):language(_language)
{
	LOG_DBG("new Parser\n");
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

	tokenizeExpressionString();

	size_t tokenId = 0;
	Member* resultValue = parseExpression(tokenId);

	if (resultValue == nullptr)
		return false;

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

				// Set the result
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

Member* Parser::parseBinaryOperationExpression(size_t& _tokenId, unsigned short _precedence, Member* _leftOverride, Member* _rightOverride) {

	Member* result = nullptr;

	const size_t tokenToEvalCount = tokens.size() - _tokenId;

	if (tokenToEvalCount < 3)
		return nullptr;

	const Token& token1(tokens.at(_tokenId));
	const Token& token2(tokens.at(_tokenId+1));
	const Token& token3(tokens.at(_tokenId+2));

	// Structure check
	const bool isValid = token1.type != TokenType_Operator &&
			                token2.type == TokenType_Operator &&
			                token3.type != TokenType_Operator;

	if (!isValid) {
		return nullptr;
	}
		 
	// Precedence check
	const auto currentOperatorPrecedence = language->getOperatorPrecedence(token2.word);
		
	if (currentOperatorPrecedence < _precedence)
		return nullptr;

	// Parse right expression
	size_t rightTokenId = _tokenId + 2;
	auto right = parseExpression(rightTokenId, currentOperatorPrecedence);

	// Build the graph for the first 3 tokens
	Container* context = this->getParent();

	Member* left = _leftOverride != nullptr ? _leftOverride : operandTokenToMember(token1);

	// Special behavior for "=" operator
	if (token2.word == "=") {

		// left operand (should BE a variable)

		NODABLE_ASSERT(left->getOwner() != nullptr); // left operand cannot be a orphaned member
		NODABLE_ASSERT(left->getOwner()->getMember("__class__")->getValueAsString() == "Variable"); // left operand need to me owned by a variable node			               


		// Directly connects right operand output to left operant input (yes that's reversed compared to code)
		if (right->getOwner() == nullptr)
			left->getOwner()->setMember("value", right);
		else
			Entity::Connect(context->createWire(), right, left);

		result = left->getOwner()->getFirstMemberWithConnection(Connection_InOut);


	// For all other binary operations :
	}else {
		auto binOperation = context->createNodeBinaryOperation(token2.word);

		// Connect the Left Operand :
		//---------------------------
		if (left->getOwner() == nullptr)
			binOperation->setMember("left", left);
		else
			Entity::Connect(context->createWire(), left, binOperation->getMember("left"));

		// Connect the Right Operand :

		if (right->getOwner() == nullptr)
			binOperation->setMember("right", right);
		else
			Entity::Connect(context->createWire(), right, binOperation->getMember("right"));

		// Set the result !
		result = binOperation->getMember("result");
	}
	
	_tokenId = rightTokenId;

	return result;
}

Member* Parser::parseUnaryOperationExpression(size_t& _tokenId, unsigned short _precedence ) {

	Member* result = nullptr;

	const bool hasEnoughtTokens = tokens.size() > _tokenId + 1;
	if ( !hasEnoughtTokens )
		return result;

	const Token& token1(tokens.at(_tokenId++));
	const Token& token2(tokens.at(_tokenId++));

	if (token1.type != TokenType_Operator) {
		_tokenId -= 2;
		return result;
	}

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

Member* Parser::parsePrimaryExpression( size_t& _tokenId) {

	// Check if there is index is not out of bounds
	if (tokens.size() <= _tokenId)
		return nullptr;

	auto token = tokens.at(_tokenId++);

	// Check if token is not an operator
	if (token.type == TokenType_Operator) {
		_tokenId--;
		return nullptr;
	}

	return operandTokenToMember(token);
}

Member* Parser::parseExpression(size_t& _tokenId, unsigned short _precedence, Member* _leftOverride, Member* _rightOverride) {

	Member*          result = nullptr;

    if (result = parseBinaryOperationExpression(_tokenId, _precedence, _leftOverride, _rightOverride)){
		LOG_DBG("Binary operation expression parsed.\n");

	} else if (result = parseUnaryOperationExpression(_tokenId, _precedence)) {
		LOG_DBG("Unary operation expression parsed.\n");

	} else if (result = parsePrimaryExpression(_tokenId)) {
		LOG_DBG("Primary expression parsed.\n");
		return result;
	}

	if ( result && _tokenId < tokens.size()) {
		_tokenId--;
		result = parseExpression(_tokenId, _precedence, result);
	}

	return result;
}


bool Parser::isSyntaxValid()
{
	bool success = true;	
	LOG_DBG("Parser::isSyntaxValid() : ");

	// only support odd token count
	if( tokens.size()%2 == 1)
	{
		// Check if even indexes are all NOT an Operator
		{
			size_t index = 0;
			while(index < tokens.size() && success)
			{
				if ( tokens[index].type == TokenType_Operator)
					success = false;
				index +=2;
			}
		}

		// Check if odd indexes are all an Operator
		{
			size_t index = 1;
			while(index < tokens.size() && success)
			{
				if ( tokens[index].type != TokenType_Operator)
					success = false;
				index +=2;
			}
		}
		
	}else{
		success = false;
	}

	if(!success)
		LOG_DBG("FAILED\n");
	else
		LOG_DBG("OK\n");

	return success;
}

void Parser::tokenizeExpressionString()
{
	LOG_DBG("Parser::tokenize() - START\n");

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
		// Term -> Number
		//---------------

		if( numbers.find(*it) != std::string::npos )
		{

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
		}		
	}
	LOG_DBG("Parser::tokenize() - DONE !\n");
}

void Parser::addToken(TokenType_  _type, std::string _string, size_t _charIndex)
{
	Token t;
	t.type      = _type;
	t.word      = _string;
	t.charIndex = _charIndex;

	LOG_DBG("Parser::addToken(%d, \"%s\", %llu)\n", _type, _string.c_str(), _charIndex);
	tokens.push_back(t);
}
