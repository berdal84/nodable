#include "Node_Lexer.h"
#include "Log.h"          // for LOG_DBG(...)

#include "Value.h"
#include "Node_Container.h"
#include "Node_Variable.h"
#include "BinaryOperationComponents.h"
#include "NodeView.h"
#include "Wire.h"

using namespace Nodable;

Node_Lexer::Node_Lexer()
{
	LOG_DBG("new Node_Lexer\n");
	setMember("class", "Node_Lexer");
	addMember("expression", Visibility_Protected);
	addMember("numbers", 	Visibility_Protected);
	addMember("letters", 	Visibility_Protected);
	addMember("operators", 	Visibility_Protected);

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
		NodeView::ArrangeRecursively((NodeView*)result->getComponent("view"));
		
		success = true;
	}else{
		success = false;
	}
	
	if ( !success )
		LOG_DBG("Node_Lexer::eval() - error, abording...\n");

	return success;
}

Value* Node_Lexer::createValueFromToken(Token token)
{
	Node_Container* context         = this->getParent();
	Value*          value           = nullptr;

	auto            tokenWordString = token.word.c_str();

	switch ( token.type )
	{
		case TokenType_Symbol:
		{
			Node_Variable* variable = context->find(tokenWordString);

			if ( variable == nullptr )
				variable = context->createNodeVariable(tokenWordString);

			value = variable->getValue();
			break;
		}

		case TokenType_Number:{
			value = new Value();
			value->setValue(std::stod(tokenWordString));
			break;
		}

		case TokenType_String:{
			value = new Value();
			value->setValue(tokenWordString);	
			break;
		}
		default:{}
	}
	return value;
}

Node_Variable* Node_Lexer::buildGraph()
{
	LOG_DBG("Node_Lexer::buildGraph() - START\n");
	
	Value*         resultValue       = buildGraphRec();
	Node_Variable* resultVariable    = this->getParent()->createNodeVariable("Result");	

	// If the value has no owner, we simplly set the variable value
	if( resultValue->getOwner() == nullptr)
		resultVariable->setValue(resultValue);
	// Else we connect resultValue with resultVariable.value
	else
		Node::Connect(new Wire(), resultValue, resultVariable->getValue());


	LOG_DBG("Node_Lexer::buildGraph() - DONE !\n");
	return resultVariable;
}

Value* Node_Lexer::buildGraphRec(size_t _tokenId, size_t _tokenCountMax, Value* _leftValueOverride, Value* _rightValueOverride)
{
	Value*          result;
	Node_Container* context = this->getParent();

	//printf("Token evaluated : %lu.\n", _tokenId);

	// Computes the number of token to eval
	size_t tokenToEvalCount = tokens.size() - _tokenId;
	if (_tokenCountMax != 0 )
		tokenToEvalCount = std::min(_tokenCountMax, tokenToEvalCount);

	/* operand */
	if ( tokenToEvalCount == 1)
	{		
		result = createValueFromToken(tokens[_tokenId]);

	/* operand, operator, operand */
	}else if  (tokenToEvalCount == 3)
	{
		std::string op    = tokens[_tokenId+1].word;

		auto binOperation = context->createNodeBinaryOperation(op);		
		
		// Set the left value
		if ( _leftValueOverride != nullptr ){
			Node::Connect(new Wire(), _leftValueOverride, binOperation->getMember("left"));			
		}else{
			auto left = buildGraphRec(_tokenId, 1);
			binOperation->setMember("left", left);
		}
		
		// Set the right value
		if ( _rightValueOverride != nullptr ){
			Node::Connect(new Wire(), _rightValueOverride, binOperation->getMember("right"));			
		}else{
			auto right = buildGraphRec(_tokenId+2, 1);
			binOperation->setMember("right", right);
		}

		result = binOperation->getMember("result");

	/* operand, operator, operand, operator, expr */
	}else if  (tokenToEvalCount >= 4)
	{	
		/* Operator precedence */
		std::string op     = tokens[_tokenId+1].word;
		std::string nextOp = tokens[_tokenId+3].word;		
		bool evaluateNow = BinaryOperationComponent::NeedsToBeEvaluatedFirst(op, nextOp);	

		if ( evaluateNow ){
			// Evaluate first 3 tokens passing the previous result
			auto intermediateResult = buildGraphRec(_tokenId, 3, _leftValueOverride);

			// Then evaluates the rest starting at id + 2
			result = buildGraphRec(_tokenId+2, 0, intermediateResult);	

		}else{
			// Then evaluates starting at id + 2
			auto intermediateResult = buildGraphRec(_tokenId+2, 0, _rightValueOverride);	

			// Build the graph for the first 3 tokens
			result = buildGraphRec(_tokenId, 3, _leftValueOverride, intermediateResult);
		}
	}

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