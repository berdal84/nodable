#include "Lexer.h"
#include "Log.h"          // for LOG_DBG(...)

#include "Value.h"
#include "Container.h"
#include "Variable.h"
#include "BinaryOperation.h"
#include "NodeView.h"
#include "Wire.h"

using namespace Nodable;

Lexer::Lexer()
{
	LOG_DBG("new Lexer\n");
	setMember("__class__", "Lexer");
	addMember("expression", Visibility_Protected);
	addMember("numbers", 	Visibility_Protected);
	addMember("letters", 	Visibility_Protected);
	addMember("operators", 	Visibility_Protected);

	setMember("numbers", "0123456789.");	
	setMember("letters", "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_");	
	setMember("operators", "+-*/=");

	setLabel("Lexer");
}

Lexer::~Lexer()
{

}

bool Lexer::eval()
{
	bool success;

	LOG_DBG("Lexer::eval() - tokenize\n");
	tokenize();
	LOG_DBG("Lexer::eval() - check syntax\n");
	if ( isSyntaxValid() )
	{
		LOG_DBG("Lexer::eval() - build tree and eval\n");
		auto result = buildGraph();
		NodeView::ArrangeRecursively(result->getComponent("view")->getAs<NodeView*>());
		
		success = true;
	}else{
		success = false;
	}
	
	if ( !success )
		LOG_DBG("Lexer::eval() - error, abording...\n");

	return success;
}

Variable* Lexer::buildGraph()
{
	LOG_DBG("Lexer::buildGraph() - START\n");	
	Value*         resultValue = buildGraphRec();
	auto           container   = this->getParent();

	LOG_DBG("Lexer::buildGraph() - Assign result to a variable.\n");
	Variable* resultVariable    = container->createNodeResult();	

	// If the value has no owner, we simplly set the variable value
	if( resultValue->getOwner() == nullptr)
		resultVariable->setValue(resultValue);
	// Else we connect resultValue with resultVariable.value
	else
		Entity::Connect(container->createWire(), resultValue, resultVariable->getValue());


	LOG_DBG("Lexer::buildGraph() - DONE !\n");
	return resultVariable;
}

Value* Lexer::buildGraphRec(size_t _tokenId, size_t _tokenCountMax, Value* _leftValueOverride, Value* _rightValueOverride)
{
	Value*          result;
	Container* context = this->getParent();
	NODABLE_ASSERT(context != nullptr);

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
		std::string tokenWordString = tokens[_tokenId].word;
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
				LOG_DBG("Symbol 1.\n");
				Variable* variable = context->find(tokenWordString);
				LOG_DBG("Symbol 2.\n");
				if ( variable == nullptr )
					variable = context->createNodeVariable(tokenWordString);
				LOG_DBG("Symbol 3 .\n");
				NODABLE_ASSERT(variable != nullptr);
				NODABLE_ASSERT(variable->getValue() != nullptr);

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

			default:
			{
				NODABLE_ASSERT(false);
			}
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
			Entity::Connect(context->createWire(), left, binOperation->getMember("left"));	
		
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
			Entity::Connect(context->createWire(), right, binOperation->getMember("right"));	

		// Set the result !
		result = binOperation->getMember("result");

	//----------------------------------------------------------------------------
	// Expression -> ( Operand , Binary Operator , Operand, Binary Operator, ... )
	//----------------------------------------------------------------------------

	}else if  (tokenToEvalCount >= 4)
	{	
		/* Operator precedence */
		std::string firstOperator  = tokens[_tokenId+1].word;
		std::string nextOperator   = tokens[_tokenId+3].word;		
		bool firstOperatorHasHigherPrecedence = BinaryOperationComponent::NeedsToBeEvaluatedFirst(firstOperator, nextOperator);	

		if ( firstOperatorHasHigherPrecedence ){
			// Evaluate first 3 tokens passing the previous result
			auto intermediateResult = buildGraphRec(_tokenId, 3, _leftValueOverride);

			// Then evaluates the rest starting at id + 2
			result = buildGraphRec(_tokenId+2, 0, intermediateResult);	

		}else{

			// build the graph for the right operand
			size_t tokenToEvaluateCount = 0;
			size_t lastOperatorIndex    = 0;
			bool   indexFound           = false;
			while( !indexFound &&
				    2 + tokenToEvaluateCount <= tokenToEvalCount)
			{
				if (tokens[_tokenId + 2 + tokenToEvaluateCount].type == TokenType_Operator)
				{

					nextOperator = tokens[_tokenId + 2 + tokenToEvaluateCount].word;
					if (BinaryOperationComponent::NeedsToBeEvaluatedFirst(firstOperator, nextOperator))
					{
						indexFound = true;
						break;
					}
				}
				tokenToEvaluateCount++;
			}
			
			auto right = buildGraphRec(_tokenId+2, tokenToEvaluateCount);	

			// Build the graph for the first 3 tokens
			auto intermediateResultLeft = buildGraphRec(_tokenId, 3, _leftValueOverride, right);

			// Then evaluates the rest if needed
			if ( _tokenId + 2 + tokenToEvaluateCount - 1 < tokens.size())
				result = buildGraphRec(_tokenId + 2 + tokenToEvaluateCount - 1, 0, intermediateResultLeft);
			else
				result = intermediateResultLeft;
		}
	}

	return result;
}


bool Lexer::isSyntaxValid()
{
	bool success = true;	
	LOG_DBG("Lexer::isSyntaxValid() : ");

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

void Lexer::tokenize()
{
	LOG_DBG("Lexer::tokenize() - START\n");

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
	LOG_DBG("Lexer::tokenize() - DONE !\n");
}

void Lexer::addToken(TokenType_  _type, std::string _string, size_t _charIndex)
{
	Token t;
	t.type      = _type;
	t.word      = _string;
	t.charIndex = _charIndex;

	LOG_DBG("Lexer::addToken(%d, \"%s\", %llu)\n", _type, _string.c_str(), _charIndex);
	tokens.push_back(t);
}