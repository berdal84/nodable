#include "Node_Lexer.h"
#include "Log.h"          // for LOG_DBG(...)
#include "Node_Number.h"
#include "Node_String.h"

using namespace Nodable;

Node_Lexer::Node_Lexer(Node_String* _expression):
expression(_expression)
{
	LOG_DBG("New Node_Lexer ready to tokenize \"%s\"\n", _expression->getValue() );
}

Node_Lexer::~Node_Lexer()
{

}

void Node_Lexer::evaluate()
{
	tokenize();
	if ( isSyntaxValid() )
	{
		buildExecutionTreeAndEvaluate();
	}
}

Node_Number* Node_Lexer::convertTokenToNode(Token token)
{
	Node_Context* context = this->getContext();

	// If it is a symbol 
	if (token.first == "symbol"){
		Node_Symbol* symbol = context->find(token.second.c_str());
		// If symbol not already exists, we create it
		if ( symbol == nullptr )
			symbol = context->createNodeSymbol(token.second.c_str(), context->createNodeNumber());
		// Return symbol value
		return (Node_Number*)symbol->getValue();

	// If it is a number
	}else if ( token.first == "number"){
		return context->createNodeNumber(token.second.c_str());
	}else {
		return nullptr;
	}
}

void Node_Lexer::buildExecutionTreeAndEvaluateRec(size_t _tokenIndex, Node_Number* _finalRes, Node_Number* _prevRes)
{

	Node_Number* 	      left;
	Node_Number*          right;
	Node_BinaryOperation* operation;
	Node_Context*         context = this->getContext();

	//printf("Token evaluated : %lu.\n", _tokenIndex);

	// If a previous result is set, it means we have already calculated the left part in the previous expression
	// So we use it as left operand

	if ( _prevRes != nullptr ){
		left = _prevRes;

	// Else, we parse the left operand
	}else{
		left = convertTokenToNode(tokens[_tokenIndex]);
	}
	

	size_t tokenLeft = tokens.size() - _tokenIndex;
	/* number */
	if ( tokenLeft == 1)
	{
		_finalRes->setValue(left->getValue());

	/* number, op, expr */
	}else if  (tokenLeft == 3){
		const char op = *tokens[_tokenIndex+1].second.c_str();
		right 	= convertTokenToNode(tokens[_tokenIndex+2]);
		buildExecutionTreeAndEvaluateRec(_tokenIndex+2, right, nullptr);		
		operation = context->createNodeBinaryOperation(op, left, right, _finalRes);
		operation->evaluate();

	/* number, op, number, op, expr */
	}else if  (tokenLeft >= 4)
	{	
		const char op     = *tokens[_tokenIndex+1].second.c_str();		
		const char nextOp = *tokens[_tokenIndex+3].second.c_str();	
		right 	= convertTokenToNode(tokens[_tokenIndex+2]);

		/* if currOperator is more important than nextOperator
		   we perform the first operation and send the result as left operand to the next expression */

		bool evaluateNow = Node_BinaryOperation::NeedsToBeEvaluatedFirst(op, nextOp);

		if ( evaluateNow ){
			// Perform the operation on the left
			Node_Number* 	result 	= context->createNodeNumber();			
			operation 		        = context->createNodeBinaryOperation(op, left, right, result);
			operation->evaluate();
			// Pass the result and build the next operations
			buildExecutionTreeAndEvaluateRec(_tokenIndex+2, _finalRes, result);	

		/* Else, we evaluate the next expression and then perform the operation with 
		the result of the next expresssion as right operand */
		}else{
			buildExecutionTreeAndEvaluateRec(_tokenIndex+2, right, nullptr);		
			operation = context->createNodeBinaryOperation( op, left, right, _finalRes);	
			operation->evaluate();
		}
	}
}


void Node_Lexer::buildExecutionTreeAndEvaluate()
{
	LOG_DBG("Node_Lexer::buildExecutionTreeAndEvaluate() - START\n");
	auto currentTokenIndex = 0;
	Node_Number* result = this->getContext()->createNodeNumber();	

	LOG_MSG("\nExecution step by step :\n");
	buildExecutionTreeAndEvaluateRec(currentTokenIndex, result, nullptr);
	LOG_DBG("Node_Lexer::buildExecutionTreeAndEvaluate() - DONE !\n");

	// Draw the execution tree :
	LOG_MSG("\nTree view :\n");
	Node::DrawRecursive(result);

	// Display the result :
	LOG_MSG("\nResult: %f", result->getValue());
}

bool Node_Lexer::isSyntaxValid()
{
	bool success = true;	
	LOG_DBG("Node_Lexer::isSyntaxValid() - START\n");


	if(!( tokens.size()%2 == 1))
	{
		for(size_t i = 0; i < tokens.size(); i=i+2){
			if ( !(tokens[i].first == "number" ||
				 tokens[i].first == "symbol"))
				success = false;
		}
		for(size_t i = 1; i < tokens.size(); i=i+2){
			if ( tokens[i].first != "operator")
				success = false;
		}
		LOG_MSG("The only syntax accepted is \"number\", \"operator\", \"number\", etc... \n");
		success = false;
	}

	if(!success)
		LOG_MSG("Node_Lexer::isSyntaxValid() - FAIL...\n");

	return success;
}

void Node_Lexer::tokenize()
{
	LOG_DBG("Node_Lexer::tokenize() - START\n");
	/* get expression chars */
	std::string chars = expression->getValue();

	/* prepare allowed chars */
	std::string numbers 	= "0123456789.";
	std::string letters		= "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
	std::string operators 	= "+-*/=";

	for(auto it = chars.begin(); it < chars.end(); ++it)
	{

		 /* Search for a number */
		/////////////////////////

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
			addToken("number", number);

		 /* Search for a string */
		/////////////////////////

		}else 	if(*it == '"')
		{
			++it;
			auto itStart = it;
			while(	it != chars.end() && *it != '"')
			{
				++it;
			}

			std::string str = chars.substr(itStart - chars.begin(), it - itStart);
			addToken("string", str);

		 /* Search for a symbol */
		/////////////////////////

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
			addToken("symbol", str);

		 /* Search for an operator */
		////////////////////////////
			
		}else 	if(operators.find(*it) != std::string::npos)
		{
			std::string str = chars.substr(it - chars.begin(), 1);
			addToken("operator", str);
		}		
	}
	LOG_DBG("Node_Lexer::tokenize() - DONE !\n");
}

void Node_Lexer::addToken(std::string _category, std::string _string)
{
	Token t(_category, _string);
	LOG_DBG("Node_Lexer::addToken() - %-10s => \"%s\" \n", ("\"" + _category + "\"").c_str(), _string.c_str() );
	tokens.push_back(t);
}