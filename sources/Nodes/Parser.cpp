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

// Enable detailed logs
// #define DEBUG_PARSER

#ifdef DEBUG_PARSER  // macro to disable these on debug
	#define LOG_DEBUG_PARSER(...) LOG_DEBUG(__VA_ARGS__)
#else
	#define LOG_DEBUG_PARSER(...)
#endif // DEBUG_PARSER

using namespace Nodable;

Parser::Parser(const Language* _language):language(_language)
{
	add("expression", OnlyWhenUncollapsed);
	setLabel("Parser");
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
	Variable* result = container->newResult();
	container->tryToRestoreResultNodePosition();

	// If the value has no owner, we simply set the variable value
	if (resultValue->getOwner() == nullptr)
		result->set(resultValue);
	// Else we connect resultValue with resultVariable.value
	else
		Node::Connect(container->newWire(), resultValue, result->getMember());


	auto view = result->getComponent<NodeView>();
	NodeView::ArrangeRecursively(view);
		
	success = true;

	return success;
}

Member* Parser::operandTokenToMember(const Token& _token) {


	Member* result = nullptr;

	switch (_token.type)
	{

		case TokenType::Bool:
		{
			result = new Member();
			const bool value = _token.word == "true";
			result->set(value);
			break;
		}

		case TokenType::Symbol:
		{
			auto context = getParent();
			Variable* variable = context->findVariable(_token.word);

			if (variable == nullptr)
				variable = context->newVariable(_token.word);

			NODABLE_ASSERT(variable != nullptr);
			NODABLE_ASSERT(variable->getMember() != nullptr);

			result = variable->getMember();

			break;
		}

		case TokenType::Double: {
			result = new Member();
			const double number = std::stod(_token.word);
			result->set(number);
			break;
		}

		case TokenType::Str: {
			result = new Member();
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
	const auto currentOperatorPrecedence = language->getOperatorPrecedence(token1.word);
		
	if (currentOperatorPrecedence <= _precedence) { // always eval the first operation if they have the same precedence or less.
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

	// Build the graph for the first 3 tokens
	Container* context = this->getParent();	

	// Special behavior for "=" operator
	if (token1.word == "=") {

		// Directly connects right operand output to left operant input (yes that's reversed compared to code)
		auto var =_left->getOwner()->as<Variable>();

		if (right->getOwner() == nullptr)
			var->set(right);
		else
			Node::Connect(context->newWire(), right, _left);

		result = var->getMember();


	// For all other binary operations :
	} else if (auto signature = language->findOperator(token1.word)) {

		auto binOperation = context->newBinOp( token1.word, *signature);

		// Connect the Left Operand :
		//---------------------------
		if (_left->getOwner() == nullptr)
			binOperation->set("left", _left);
		else
			Node::Connect(context->newWire(), _left, binOperation->get("left"));

		// Connect the Right Operand :

		if (right->getOwner() == nullptr)
			binOperation->set("right", right);
		else
			Node::Connect(context->newWire(), right, binOperation->get("right"));

		// Set the left !
		result = binOperation->get("result");

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
	const Token& token2(tokens.at(_tokenId+1));

	// Check if we get an operator first
	if (token1.type != TokenType::Operator) {
		LOG_DEBUG_PARSER("parseUnaryOperationExpression... " KO " (operator not found)\n");
		return nullptr;
	}

	// Then check if the operator can be applied to the next token
	if (token1.word == "-" && token2.type == TokenType::Double) { // TODO: create the unary operation "negates"
		result = operandTokenToMember(token2);
		result->set(-(double)*result);

	} else if (token1.word == "!" && token2.type == TokenType::Bool) { // TODO: create the unary operation "not"
		result = operandTokenToMember(token2);
		result->set(!(bool)*result);

	} else {
		LOG_DEBUG_PARSER("parseUnaryOperationExpression... " KO " (unrecognysed operator)\n");	
		return nullptr;
	}

	_tokenId += 2;
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

	_tokenId++;

	LOG_DEBUG_PARSER("parseAtomicExpression... " OK "\n");

	return operandTokenToMember(token);
}

Member* Parser::parseParenthesisExpression(size_t& _tokenId) {

	LOG_DEBUG_PARSER("parseParenthesisExpression...");
	LOG_DEBUG_PARSER("%s \n", Parser::logTokens(tokens, _tokenId).c_str());

	if (_tokenId >= tokens.size())
		return nullptr;

	auto token1(tokens.at(_tokenId));

	if (token1.type != TokenType::Bracket) {
		return nullptr;
	}

	Member* result(nullptr);

	if (token1.word == "(") {

		auto subToken = _tokenId + 1;
		result = parseExpression(subToken, 0u, nullptr);
		_tokenId = subToken + 1;

		if ( tokens.at(subToken).word != ")" ) {
			LOG_DEBUG_PARSER("%s \n", Parser::logTokens(tokens, _tokenId).c_str());
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
		case TokenType::Bracket:
		{
			const bool isOpenParenthesis = current.word == "(";
			openedParenthesisCount += isOpenParenthesis ? 1 : -1; // increase / decrease the opened parenthesis count.

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

bool Parser::tokenizeExpressionString()
{

	/* get expression chars */
	auto chars = (std::string)*get("expression");

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
		// Term -> Double
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
			addToken(TokenType::Double, number, std::distance(chars.begin(), itStart) );
			
		//----------------
		// Term -> Str
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

				if (it == chars.end()) {
					return false;
				}
				
				std::string str = chars.substr(itStart - chars.begin(), it - itStart);
				addToken(TokenType::Str, str, std::distance(chars.begin(), itStart));

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
				addToken(TokenType::Symbol, str, std::distance(chars.begin(), itStart));

		//-----------------
		// Term -> Operator
		//-----------------
			
		}else 	if(operators.find(*it) != std::string::npos)
		{
			std::string str = chars.substr(it - chars.begin(), 1);
			addToken(TokenType::Operator, str, std::distance(chars.begin(), it));

		//-----------------
		// Term -> Parenthesis
		//-----------------
		} else if (*it == ')' || *it == '(')
		{
			std::string str = chars.substr(it - chars.begin(), 1);
			addToken(TokenType::Bracket, str, std::distance(chars.begin(), it));

		}else if (*it == ',') {
			std::string str = chars.substr(it - chars.begin(), 1);
			addToken(TokenType::Comma, str, std::distance(chars.begin(), it));

		}else if (*it == '\t') { // ignore tabs			

		}else if ( *it != ' ') {
			LOG_DEBUG_PARSER("Unable to tokenize expression %s \n", chars);
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

	// Check if the minimum token count required is available ( 0: identifier, 1: open parenthesis, 2: close parenthesis)
	if (localTokenId + 2 >= tokens.size()) {
		LOG_DEBUG_PARSER("parseFunctionCall aborted. Not enough tokens.");
		return nullptr;
	}

	// Check if starts with a symbol followed by "("
	if ( tokens.at(localTokenId).type != TokenType::Symbol ||
		 tokens.at(localTokenId+1).word != "(" ) {

		LOG_DEBUG_PARSER("parseFunctionCall aborted. Symbol + \"(\" not found...");
		return nullptr;
	}

	std::vector<Member*> argAsMember;

	// Declare a new function prototype
	auto identifier = tokens.at(localTokenId++).word;
	FunctionSignature signature(identifier, TokenType::Unknown);

	localTokenId++; // eat parenthesis
	
	bool parsingError = false;
	while ( !parsingError && localTokenId < tokens.size() && tokens.at(localTokenId).word != ")") {

		if (auto member = parseExpression(localTokenId)) {
			argAsMember.push_back(member); // store argument as member (already parsed)
			signature.pushArg( Member::MemberTypeToTokenType(member->getType()) );  // add a new argument type to the proto.

			if (tokens.at(localTokenId).type == TokenType::Comma)
				localTokenId++;

		} else {
			parsingError = true;
		}
	}


	if (tokens.at(localTokenId).word != ")") {
		LOG_DEBUG_PARSER("parseFunctionCall aborted. Close parenthesis expected !");
		return nullptr;
	}

	localTokenId++; // eat parenthesis

	// Find the prototype in the language library
	auto fct = language->findFunction(signature);

	if( fct != nullptr) { // if function found

		_tokenId = localTokenId;
		Container* context = this->getParent();

		auto node = context->newFunction(*fct);

		auto connectArg = [&](size_t _argIndex)-> void { // lambda to connect input member to node for a specific argument index.

			auto arg = argAsMember.at(_argIndex);
			auto memberName = fct->signature.getArgs().at(_argIndex).name;

			if (arg->getOwner() == nullptr) {
				node->set(memberName.c_str(), arg);
			} else {
				Node::Connect(context->newWire(), arg, node->get(memberName.c_str()));
			}
		};

		for( size_t argIndex = 0; argIndex < fct->signature.getArgs().size(); argIndex++ )
			connectArg(argIndex);

		return node->get("result");

	} else {
		LOG_DEBUG_PARSER("Unable to parse function, prototype not found.");
	}

	return nullptr;
}
