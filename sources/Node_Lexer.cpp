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

	//----------------------
	// Expression -> Operand
	//----------------------

	if ( tokenToEvalCount == 1)
	{		
		auto tokenWordString = tokens[_tokenId].word;
		switch ( tokens[_tokenId].type )
		{
			//--------------------
			// Operand -> Boolean
			//--------------------

			case TokenType_Boolean:
			{
				result = new Value();	
				result->setValue(tokenWordString == "true");

				break;
			}

			//--------------------
			// Operand -> Symbol
			//--------------------

			case TokenType_Symbol:
			{
				Node_Variable* variable = context->find(tokenWordString.c_str());

				if ( variable == nullptr )
					variable = context->createNodeVariable(tokenWordString.c_str());

				result = variable->getValue();
				
				break;
			}

			//--------------------
			// Operand -> Number
			//--------------------

			case TokenType_Number:{
				result = new Value();
				result->setValue(std::stod(tokenWordString));
				break;
			}

			//--------------------
			// Operand -> String
			//--------------------

			case TokenType_String:{
				result = new Value();
				result->setValue(tokenWordString);	
				break;
			}
			default:{}
		}

	//-------------------------------------------
	// Expression -> ( Unary Operator , Operand )
	//-------------------------------------------

	//}else if (tokenToEvalCount == 2)
	//{
		// TODO

	//------------------------------------------------------
	// Expression -> ( Operand , Binary Operator , Operand )
	//------------------------------------------------------

	}else if  (tokenToEvalCount == 3)
	{
		std::string op    = tokens[_tokenId+1].word;

		auto binOperation = context->createNodeBinaryOperation(op);		
		
		// Connect the Left Operand :
		//---------------------------

		Value* left;
		if ( _leftValueOverride != nullptr )
			left = _leftValueOverride;			
		else
			left = buildGraphRec(_tokenId, 1);

		if (left->getOwner() == nullptr)
			binOperation->setMember("left", left);
		else
			Node::Connect(new Wire(), left, binOperation->getMember("left"));	
		
		// Connect the Right Operand :
		//----------------------------

		Value* right;
		if ( _rightValueOverride != nullptr )
			right = _rightValueOverride;			
		else
			right = buildGraphRec(_tokenId+2, 1);

		if (right->getOwner() == nullptr)
			binOperation->setMember("right", right);
		else
			Node::Connect(new Wire(), right, binOperation->getMember("right"));	

		// Set the result !
		result = binOperation->getMember("result");

	//----------------------------------------------------------------------------
	// Expression -> ( Operand , Binary Operator , Operand, Binary Operator, ... )
	//----------------------------------------------------------------------------

	}else if  (tokenToEvalCount >= 4)
	{	
		/* Operator precedence */
		std::string firstOperator  = tokens[_tokenId+1].word;
		std::string secondOperator = tokens[_tokenId+3].word;		
		bool firstOperatorHasHigherPrecedence = BinaryOperationComponent::NeedsToBeEvaluatedFirst(firstOperator, secondOperator);	

		if ( firstOperatorHasHigherPrecedence ){
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

	/* prepare reserved keywords */
	std::map<std::string, TokenType_> keywords;
	keywords["true"]  = TokenType_Boolean;
	keywords["false"] = TokenType_Boolean;

	for(auto it = chars.begin(); it < chars.end(); ++it)
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
			
			if ( it != chars.end())
				--it;

			std::string number = chars.substr(itStart - chars.begin(), it - itStart + 1);
			addToken(TokenType_Number, number, std::distance(chars.begin(), itStart) );
			
		//----------------
		// Term -> String
		//----------------

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
				addToken(keywords[str], str, std::distance(chars.begin(), itStart));

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