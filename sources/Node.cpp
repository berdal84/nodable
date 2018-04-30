#include "Node.h"
#include "Log.h"		// for LOG_DBG(...)
#include <algorithm>    // std::find_if
#include <cstring>      // for strcmp
#include "Node_Value.h"
#include "Node_Number.h"
#include "Node_String.h"

using namespace Nodable;

 // Node :
//////////

Node::Node(){}
Node::~Node(){}

Node_Context* Node::getContext()const
{
	return this->context;
}

void Node::setContext(Node_Context* _context)
{
	this->context = _context;
}

Node* Node::getInput  (size_t _id)const
{
	return input.at(_id);
}

Node* Node::getOutput (size_t _id)const
{
	return output.at(_id);
}

void Node::setInput  (Node* _node, size_t _id)
{
	// Resizes if needed
	if ( input.size() <= _id) input.resize(_id + 1);
	
	// Set the input
	input.at(_id) = _node;
}

void Node::setOutput (Node* _node, size_t _id)
{
	// Resizes if needed
	if ( output.size() <= _id) output.resize(_id + 1);
	
	// Set the input
	output.at(_id) = _node;
}


void Node::DrawRecursive(Node* _node, std::string _prefix)
{
	if ( _node == nullptr)
		return;	

	// Draw its inputs
	for(auto each : _node->input)
	{
		DrawRecursive(each, _prefix + "  ");
	}
	
	_prefix = _prefix + "  ";
	
	// Draw the node
	printf("%s", _prefix.c_str());
	_node->draw();
	printf("\n");
}

 // Node_BinaryOperation :
//////////////////////////

Node_BinaryOperation::Node_BinaryOperation(	Node_Value* _leftInput,
											Node_Value* _rightInput,
											Node_Value* _output):
	leftInput(_leftInput),
	rightInput(_rightInput),
	output(_output)
{
	// Connects the left input  (from both sides) 
	this->setInput (_leftInput , 0);
	_leftInput->setOutput(this);

	// Connects the right input (from both sides)
	this->setInput (_rightInput, 1);
	_rightInput->setOutput(this);

	// Connects the output      (from both sides)
	this->setOutput(_output);
	_output->setInput(this);
}

Node_BinaryOperation::~Node_BinaryOperation()
{

}

Node_Value* Node_BinaryOperation::getLeftInput()const
{
	return leftInput;
}

Node_Value* Node_BinaryOperation::getRightInput()const
{
	return rightInput;
}

Node_Value* Node_BinaryOperation::getOutput()const
{
	return output;
}

/* Precendence for binary operators */
bool Node_BinaryOperation::NeedsToBeEvaluatedFirst(const char op, const char nextOp)
{
	if (op == '=' && nextOp == '=') return false;	
	if (op == '=' && nextOp == '-') return false;	
	if (op == '=' && nextOp == '+') return false;	
	if (op == '=' && nextOp == '*') return false;	
	if (op == '=' && nextOp == '/') return false;

	if (op == '+' && nextOp == '=') return false;
	if (op == '+' && nextOp == '-') return true;	
	if (op == '+' && nextOp == '+') return true;	
	if (op == '+' && nextOp == '*') return false;	
	if (op == '+' && nextOp == '/') return false;

	if (op == '-' && nextOp == '=') return false;
	if (op == '-' && nextOp == '-') return true;	
	if (op == '-' && nextOp == '+') return true;	
	if (op == '-' && nextOp == '*') return false;	
	if (op == '-' && nextOp == '/') return false;

	if (op == '*' && nextOp == '=') return false;
	if (op == '*' && nextOp == '-') return true;	
	if (op == '*' && nextOp == '+') return true;	
	if (op == '*' && nextOp == '*') return true;	
	if (op == '*' && nextOp == '/') return true;

	if (op == '/' && nextOp == '=') return false;
	if (op == '/' && nextOp == '-') return true;	
	if (op == '/' && nextOp == '+') return true;	
	if (op == '/' && nextOp == '*') return true;	
	if (op == '/' && nextOp == '/') return true;

	return true;
}

 // Node_Add :
//////////////

Node_Add::Node_Add(	Node_Value* _leftInput,
					Node_Value* _rightInput,
					Node_Value* _output):
	Node_BinaryOperation(_leftInput, _rightInput, _output)
{

}

Node_Add::~Node_Add()
{

}

void Node_Add::evaluate()
{
	double result = this->getLeftInput()->asNumber()->getValue() + this->getRightInput()->asNumber()->getValue();
	LOG_MSG("%f + %f = %f", this->getLeftInput()->asNumber()->getValue(), this->getRightInput()->asNumber()->getValue(), result);
	this->getOutput()->asNumber()->setValue(result);
}

 // Node_Substract :
///////////////////////

Node_Substract::Node_Substract(	Node_Value* _leftInput,
					Node_Value* _rightInput,
					Node_Value* _output):
	Node_BinaryOperation( _leftInput, _rightInput, _output)
{

}

Node_Substract::~Node_Substract()
{

}

void Node_Substract::evaluate()
{
	double result = this->getLeftInput()->asNumber()->getValue() - this->getRightInput()->asNumber()->getValue();
	LOG_MSG("%f - %f = %f", this->getLeftInput()->asNumber()->getValue(), this->getRightInput()->asNumber()->getValue(), result);
	this->getOutput()->asNumber()->setValue(result);
}

 // Node_Divide :
///////////////////////

Node_Divide::Node_Divide(	Node_Value* _leftInput,
					Node_Value* _rightInput,
					Node_Value* _output):
	Node_BinaryOperation( _leftInput, _rightInput, _output)
{

}

Node_Divide::~Node_Divide()
{

}

void Node_Divide::evaluate()
{
	double result = this->getLeftInput()->asNumber()->getValue() / this->getRightInput()->asNumber()->getValue();
	LOG_MSG("%f / %f = %f", this->getLeftInput()->asNumber()->getValue(), this->getRightInput()->asNumber()->getValue(), result);
	this->getOutput()->asNumber()->setValue(result);
}

 // Node_Multiply :
///////////////////////

Node_Multiply::Node_Multiply(	Node_Value* _leftInput,
					Node_Value* _rightInput,
					Node_Value* _output):
	Node_BinaryOperation( _leftInput, _rightInput, _output)
{

}

Node_Multiply::~Node_Multiply()
{

}

void Node_Multiply::evaluate()
{
	double result = this->getLeftInput()->asNumber()->getValue() * this->getRightInput()->asNumber()->getValue();
	LOG_MSG("%f * %f = %f", this->getLeftInput()->asNumber()->getValue(), this->getRightInput()->asNumber()->getValue(), result);
	this->getOutput()->asNumber()->setValue(result);
}

 // Node_Multiply :
///////////////////////

Node_Assign::Node_Assign(	Node_Value* _leftInput,
					        Node_Value* _rightInput,
					        Node_Value* _output):
	Node_BinaryOperation( _leftInput, _rightInput, _output)
{

}

Node_Assign::~Node_Assign()
{

}

void Node_Assign::evaluate()
{
	if ( this->getLeftInput()->getType() != this->getRightInput()->getType()){
		LOG_DBG("unable to assign with two different value types");
		exit(1);
	}

	if ( this->getRightInput()->getType() == Type_Number){
		auto result = this->getRightInput()->asNumber()->getValue();
		this->getLeftInput()->asNumber()->setValue(result);
		this->getOutput()   ->asNumber()->setValue(result);
	}	
}

 // Node_Symbol :
//////////////

Node_Symbol::Node_Symbol(const char* _name, Node* _value):
	name(_name),
	value(_value)
{
	LOG_DBG("New Node_Symbol : %s", _name);
}

Node_Symbol::~Node_Symbol()
{

}

const char* Node_Symbol::getName()const
{
	return name.c_str();
}

Node* Node_Symbol::getValue()const
{
	return this->value;
}

 // Node_Context :
//////////////////

Node_Context::Node_Context(const char* _name):
name(_name)
{
	LOG_DBG("A new context named '%s' has been created.", _name);
}


void Node_Context::addNode(Node* _node)
{
	/* Add the node to the node vector list*/
	this->nodes.push_back(_node);

	/* Set the node's context to this */
	_node->setContext(this);

	LOG_DBG("A node has been added to the context '%s'", this->getName());
}

Node_Symbol* Node_Context::find(const char* _name)
{
	LOG_DBG("Searching node '%s' in context '%s' : ", _name, this->getName());

	auto findFunction = [_name](const Node_Symbol* _node ) -> bool
	{
		return strcmp(_node->getName(), _name) == 0;
	};

	auto it = std::find_if(symbols.begin(), symbols.end(), findFunction);
	if (it != symbols.end()){
		LOG_DBG("FOUND !");
		return *it;
	}
	LOG_DBG("NOT found...");
	return nullptr;
}

Node_Symbol* Node_Context::createNodeSymbol(const char* _name, Node_Value* _value)
{
	Node_Symbol* node = new Node_Symbol(_name, _value);
	addNode(node);
	this->symbols.push_back(node);
	return node;
}

Node_Number*          Node_Context::createNodeNumber(int _value =0)
{
	Node_Number* node = new Node_Number(_value);
	addNode(node);
	return node;
}

Node_Number*          Node_Context::createNodeNumber(const char* _value)
{
	Node_Number* node = new Node_Number(_value);
	addNode(node);
	return node;
}

Node_String*          Node_Context::createNodeString(const char* _value = "")
{
	Node_String* node = new Node_String(_value);
	addNode(node);
	return node;
}

Node_Add* Node_Context::createNodeAdd(Node_Value* _inputA, Node_Value* _inputB, Node_Value* _output)
{
	return (Node_Add*)this->createNodeBinaryOperation('+', _inputA, _inputB, _output );
}

Node_Substract* Node_Context::createNodeSubstract(Node_Value* _inputA, Node_Value* _inputB, Node_Value* _output)
{
	return (Node_Substract*)this->createNodeBinaryOperation('-', _inputA, _inputB, _output );
}

Node_Multiply* Node_Context::createNodeMultiply(Node_Value* _inputA, Node_Value* _inputB, Node_Value* _output)
{
	return (Node_Multiply*)this->createNodeBinaryOperation('*', _inputA, _inputB, _output );
}

Node_Divide* Node_Context::createNodeDivide(Node_Value* _inputA, Node_Value* _inputB, Node_Value* _output)
{
	return (Node_Divide*)this->createNodeBinaryOperation('/', _inputA, _inputB, _output );
}

Node_Assign* Node_Context::createNodeAssign(Node_Value* _inputA, Node_Value* _inputB, Node_Value* _output)
{
	return (Node_Assign*)this->createNodeBinaryOperation('=', _inputA, _inputB, _output);
}


Node_Lexer* Node_Context::createNodeLexer           (Node_String* _input)
{
	Node_Lexer* lexer = new Node_Lexer(_input);
	addNode(lexer);
	return lexer;
}

Node_BinaryOperation* Node_Context::createNodeBinaryOperation(   
	                            const char _operator, 
								Node_Value* _leftInput, 
								Node_Value* _rightInput, 
								Node_Value* _output)
{
	Node_BinaryOperation* node = nullptr;

	if ( _operator == '+')
		node = new Node_Add(_leftInput, _rightInput, _output);
	else if ( _operator == '-')
		node = new Node_Substract(_leftInput, _rightInput, _output);
	else if ( _operator == '/')
		node = new Node_Divide(_leftInput, _rightInput, _output);
	else if ( _operator == '*')
		node = new Node_Multiply(_leftInput, _rightInput, _output);
	else if ( _operator == '=')
		node = new Node_Assign(_leftInput, _rightInput, _output);

	addNode(node);

	return node;
}

const char* Node_Context::getName()const
{
	return name.c_str();
}

 // Node_Lexer :
////////////////

Node_Lexer::Node_Lexer(Node_String* _expression):
expression(_expression)
{
	//printf("New Node_Lexer ready to tokenize \"%s\"\n", _expression->getValue() );
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
	//printf("Node_Lexer::buildExecutionTreeAndEvaluate() - START\n");
	auto currentTokenIndex = 0;
	Node_Number* result = this->getContext()->createNodeNumber();	

	LOG_MSG("\nExecution step by step :\n");
	buildExecutionTreeAndEvaluateRec(currentTokenIndex, result, nullptr);
	//printf("Node_Lexer::buildExecutionTreeAndEvaluate() - DONE !\n");

	// Draw the execution tree :
	LOG_MSG("\nTree view :\n");
	Node::DrawRecursive(result);

	// Display the result :
	LOG_MSG("\nResult: %f", result->getValue());
}

bool Node_Lexer::isSyntaxValid()
{
	bool success = true;	
	//printf("Node_Lexer::isSyntaxValid() - START\n");


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
	//printf("Node_Lexer::tokenize() - START\n");
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
	//printf("Node_Lexer::tokenize() - DONE !\n");
}

void Node_Lexer::addToken(std::string _category, std::string _string)
{
	Token t(_category, _string);
	//printf("Node_Lexer::addToken() - %-10s => \"%s\" \n", ("\"" + _category + "\"").c_str(), _string.c_str() );
	tokens.push_back(t);
}