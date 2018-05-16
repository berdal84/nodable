#include "Node_Lexer.h"
#include "Log.h"          // for LOG_DBG(...)

#include "Value.h"
#include "Node_Container.h"
#include "Node_Variable.h"
#include "Node_BinaryOperations.h"
#include "NodeView.h"

using namespace Nodable;

Node_Lexer::Node_Lexer()
{
	LOG_DBG("new Node_Lexer\n");

	addMember("expression");
	addMember("numbers" );
	addMember("letters" );
	addMember("operators" );

	setMember("numbers", "0123456789.");	
	setMember("letters", "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_");	
	setMember("operators", "+-*/=");

	setLabel("Lexer");
}

Node_Lexer::~Node_Lexer()
{

}

bool Node_Lexer::eval()
{
	bool success;

	LOG_DBG("Node_Lexer::eval() - tokenize\n");
	tokenize();
	LOG_DBG("Node_Lexer::eval() - check syntax\n");
	if ( isSyntaxValid() )
	{
		LOG_DBG("Node_Lexer::eval() - build tree and eval\n");
		auto result = buildGraph();
		NodeView::ArrangeRecursive(result->getView());
		
		success = true;
	}else{
		success = false;
	}
	
	if ( !success )
		LOG_DBG("Node_Lexer::eval() - error, abording...\n");

	return success;
}

Node_Variable* Node_Lexer::convertTokenToNode(Token token)
{
	Node_Container* context         = this->getParent();
	Node_Variable*  variable        = nullptr;

	auto            tokenWordString = token.word.c_str();

	switch ( token.type )
	{
		case TokenType_Symbol:
		{
			variable = context->find(tokenWordString);

			if ( variable == nullptr )
				variable = context->createNodeVariable(tokenWordString);

			break;
		}

		case TokenType_Number:{
			variable = context->createNodeVariable(); // That could be strange to store a constant into a variable. But A node number could be modified via the GUI.
			variable->setValue(std::stod(tokenWordString));
			break;
		}

		case TokenType_String:{
			variable = context->createNodeVariable();
			variable->setValue(tokenWordString);	
			break;
		}
		default:{}
	}
	return variable;
}

void Node_Lexer::buildGraphRec(size_t _tokenIndex, Node_Variable* _finalRes, Node_Variable* _prevRes)
{

	Node_Variable* 	      left;
	Node_Variable*        right;
	Node_Container*       context = this->getParent();

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
		_finalRes = left;

	/* number, op, expr */
	}else if  (tokenLeft == 3){
		std::string op = tokens[++_tokenIndex].word;

		right 	       = convertTokenToNode(tokens[++_tokenIndex]);
		buildGraphRec(_tokenIndex, right, nullptr);		
		context->createNodeBinaryOperation(op, left, right, _finalRes);

	/* number, op, number, op, expr */
	}else if  (tokenLeft >= 4)
	{	
		std::string op     = tokens[++_tokenIndex].word;
		right 	           = convertTokenToNode(tokens[++_tokenIndex]);
		
		// peek the next operator
		std::string nextOp = tokens[_tokenIndex+1].word;	


		/* if currOperator is more important than nextOperator
		   we perform the first operation and send the result as left operand to the next expression */
		Node_Variable* 	result 	    = context->createNodeVariable();	
		bool            evaluateNow = Node_BinaryOperation::NeedsToBeEvaluatedFirst(op, nextOp);	

		if ( evaluateNow ){
			// create the operation on the left		
			context->createNodeBinaryOperation(op, left, right, result);

			// Pass the result and build the next operations
			buildGraphRec(_tokenIndex, _finalRes, result);	

		/* Else, we evaluate the next expression and then perform the operation with 
		the result of the next expresssion as right operand */
		}else{
			buildGraphRec(_tokenIndex, result, nullptr);		
			context->createNodeBinaryOperation( op, left, result, _finalRes);	
		}
	}
}


Node_Variable* Node_Lexer::buildGraph()
{
	LOG_DBG("Node_Lexer::buildGraph() - START\n");
	auto currentTokenIndex = 0;
	Node_Variable* result = this->getParent()->createNodeVariable("Result");	
	buildGraphRec(currentTokenIndex, result, nullptr);	
	LOG_DBG("Node_Lexer::buildGraph() - DONE !\n");
	return result;
}

bool Node_Lexer::isSyntaxValid()
{
	bool success = true;	
	LOG_DBG("Node_Lexer::isSyntaxValid() - START\n");

	// only support even count token
	if( tokens.size()%2 == 1)
	{
		// with an alternance of Number|Symbol / Operator / Number|Symbol / etc.
		for(size_t i = 0; i < tokens.size(); i=i+2){
			if ( !(tokens[i].type == TokenType_Number || tokens[i].type == TokenType_Symbol))
				success = false;
		}

		for(size_t i = 1; i < tokens.size(); i=i+2){
			if ( tokens[i].type != TokenType_Operator)
				success = false;
		}
		
	}else{
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
	std::string chars = getMember("expression")->getValueAsString();

	/* prepare allowed chars */
	std::string numbers 	     = getMember("numbers")->getValueAsString();
	std::string letters		     = getMember("letters")->getValueAsString();
	std::string operators 	     = getMember("operators")->getValueAsString();

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
			
			if ( it != chars.end())
				--it;

			std::string number = chars.substr(itStart - chars.begin(), it - itStart + 1);
			addToken(TokenType_Number, number, std::distance(chars.begin(), itStart) );
			
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
			addToken(TokenType_String, str, std::distance(chars.begin(), itStart));

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
			addToken(TokenType_Symbol, str, std::distance(chars.begin(), itStart));

		 /* Search for an operator */
		////////////////////////////
			
		}else 	if(operators.find(*it) != std::string::npos)
		{
			std::string str = chars.substr(it - chars.begin(), 1);
			addToken(TokenType_Operator, str, std::distance(chars.begin(), it));
		}		
	}
	LOG_DBG("Node_Lexer::tokenize() - DONE !\n");
}

void Node_Lexer::addToken(TokenType_  _type, std::string _string, size_t _charIndex)
{
	Token t;
	t.type      = _type;
	t.word      = _string;
	t.charIndex = _charIndex;

	LOG_DBG("Node_Lexer::addToken(%d, \"%s\", %llu)\n", _type, _string.c_str(), _charIndex);
	tokens.push_back(t);
}