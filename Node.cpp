#include "Node.h"
#include "iostream"		// cout
#include <algorithm>    // std::find_if
#include <stdio.h>		// printf("%s\n", );

using namespace Nodable;
using namespace std;

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

 // Node_Number : (note: derives template Node_Value)
//////////////////
Node_Number::~Node_Number(){}
Node_Number::Node_Number():Node_Value(0.0){}
Node_Number::Node_Number(int _n):Node_Value(double(_n)){}
Node_Number::Node_Number(std::string _string):Node_Value(std::stod(_string)){}


 // Node_String :
//////////////////

Node_String::Node_String(const char* _value):
value(_value)
{
	//cout <<  "New Node_String : " << _value << endl;
}

Node_String::~Node_String(){}

void Node_String::setValue(const char* _value)
{
	//cout <<  "Node_String " <<  this->value << " becomes " << _value << endl;
	this->value = _value;
}

const char* Node_String::getValue()const
{
	return this->value.c_str();
}

 // Node_BinaryOperation :
//////////////////////////

Node_BinaryOperation::Node_BinaryOperation(	Node_Number* _leftInput,
											Node_Number* _rightInput,
											Node_Number* _output):
	leftInput(_leftInput),
	rightInput(_rightInput),
	output(_output)
{

}

Node_BinaryOperation::~Node_BinaryOperation()
{

}

Node_Number* Node_BinaryOperation::getLeftInput()const
{
	return leftInput;
}

Node_Number* Node_BinaryOperation::getRightInput()const
{
	return rightInput;
}

Node_Number* Node_BinaryOperation::getOutput()const
{
	return output;
}

/* Precendence for binary operators */
bool Node_BinaryOperation::NeedsToBeEvaluatedFirst(const char op, const char nextOp)
{
	if (op == '=' & nextOp == '=') return false;	
	if (op == '=' & nextOp == '-') return false;	
	if (op == '=' & nextOp == '+') return false;	
	if (op == '=' & nextOp == '*') return false;	
	if (op == '=' & nextOp == '/') return false;

	if (op == '+' & nextOp == '=') return false;
	if (op == '+' & nextOp == '-') return true;	
	if (op == '+' & nextOp == '+') return true;	
	if (op == '+' & nextOp == '*') return false;	
	if (op == '+' & nextOp == '/') return false;

	if (op == '-' & nextOp == '=') return false;
	if (op == '-' & nextOp == '-') return true;	
	if (op == '-' & nextOp == '+') return true;	
	if (op == '-' & nextOp == '*') return false;	
	if (op == '-' & nextOp == '/') return false;

	if (op == '*' & nextOp == '=') return false;
	if (op == '*' & nextOp == '-') return true;	
	if (op == '*' & nextOp == '+') return true;	
	if (op == '*' & nextOp == '*') return true;	
	if (op == '*' & nextOp == '/') return true;

	if (op == '/' & nextOp == '=') return false;
	if (op == '/' & nextOp == '-') return true;	
	if (op == '/' & nextOp == '+') return true;	
	if (op == '/' & nextOp == '*') return true;	
	if (op == '/' & nextOp == '/') return true;

	return true;
}

 // Node_Add :
//////////////

Node_Add::Node_Add(	Node_Number* _leftInput,
					Node_Number* _rightInput,
					Node_Number* _output):
	Node_BinaryOperation(_leftInput, _rightInput, _output)
{

}

Node_Add::~Node_Add()
{

}

void Node_Add::evaluate()
{
	double result = this->getLeftInput()->getValue() + this->getRightInput()->getValue();
	cout <<  "Node_Add:evaluate(): " <<  this->getLeftInput()->getValue() << " + " << this->getRightInput()->getValue() << " (result " << result << ")" <<endl;
	this->getOutput()->setValue(result);
}

 // Node_Substract :
///////////////////////

Node_Substract::Node_Substract(	Node_Number* _leftInput,
					Node_Number* _rightInput,
					Node_Number* _output):
	Node_BinaryOperation( _leftInput, _rightInput, _output)
{

}

Node_Substract::~Node_Substract()
{

}

void Node_Substract::evaluate()
{
	double result = this->getLeftInput()->getValue() - this->getRightInput()->getValue();
	cout <<  "Node_Substract:evaluate(): " <<  this->getLeftInput()->getValue() << " - " << this->getRightInput()->getValue() << " (result " << result << ")"<< endl;
	this->getOutput()->setValue(result);
}

 // Node_Divide :
///////////////////////

Node_Divide::Node_Divide(	Node_Number* _leftInput,
					Node_Number* _rightInput,
					Node_Number* _output):
	Node_BinaryOperation( _leftInput, _rightInput, _output)
{

}

Node_Divide::~Node_Divide()
{

}

void Node_Divide::evaluate()
{
	double result = this->getLeftInput()->getValue() / this->getRightInput()->getValue();
	cout <<  "Node_Divide:evaluate(): " <<  this->getLeftInput()->getValue() << " / " << this->getRightInput()->getValue() << "(result " << result <<")"<< endl;
	this->getOutput()->setValue(result);
}

 // Node_Multiply :
///////////////////////

Node_Multiply::Node_Multiply(	Node_Number* _leftInput,
					Node_Number* _rightInput,
					Node_Number* _output):
	Node_BinaryOperation( _leftInput, _rightInput, _output)
{

}

Node_Multiply::~Node_Multiply()
{

}

void Node_Multiply::evaluate()
{
	double result = this->getLeftInput()->getValue() * this->getRightInput()->getValue();
	cout <<  "Node_Multiply:evaluate(): " <<  this->getLeftInput()->getValue() << " * " << this->getRightInput()->getValue() << "(result " << result << ")" << endl;
	this->getOutput()->setValue(result);
}

 // Node_Multiply :
///////////////////////

Node_Assign::Node_Assign(	Node_Number* _leftInput,
					        Node_Number* _rightInput,
					        Node_Number* _output):
	Node_BinaryOperation( _leftInput, _rightInput, _output)
{

}

Node_Assign::~Node_Assign()
{

}

void Node_Assign::evaluate()
{
	double result = this->getRightInput()->getValue();
	cout <<  "Node_Multiply:evaluate(): " <<  this->getLeftInput()->getValue() << " = " << this->getRightInput()->getValue() << "(result " << result << ")" << endl;
	this->getLeftInput()->setValue(result);
	this->getOutput()->setValue(result);
}

 // Node_Symbol :
//////////////

Node_Symbol::Node_Symbol(const char* _name, Node* _value):
	name(_name),
	value(_value)
{
	//cout << "New Node_Symbol : " << _name << endl;
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

}

Node_Context::~Node_Context()
{

}

void Node_Context::addNode(Node* _node)
{
	/* Add the node to the node vector list*/
	this->nodes.push_back(_node);

	/* Set the node's context to this */
	_node->setContext(this);
}

Node_Symbol* Node_Context::find(const char* _name)
{
	//printf("Searching node with name '%s' in context named '%s' : ", _name, this->name.c_str());

	auto findFunction = [_name](const Node_Symbol* _node ) -> bool
	{
		return strcmp(_node->getName(), _name) == 0;
	};

	auto it = std::find_if(symbols.begin(), symbols.end(), findFunction);
	if (it != symbols.end()){
		return *it;
	}
	//cout << "NOT found !" << endl;
	return nullptr;
}

Node_Symbol* Node_Context::createNodeSymbol(const char* _name, Node_Number* _value)
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

Node_Add* Node_Context::createNodeAdd(Node_Number* _inputA, Node_Number* _inputB, Node_Number* _output)
{
	return (Node_Add*)this->createNodeBinaryOperation('+', _inputA, _inputB, _output );
}

Node_Substract* Node_Context::createNodeSubstract(Node_Number* _inputA, Node_Number* _inputB, Node_Number* _output)
{
	return (Node_Substract*)this->createNodeBinaryOperation('-', _inputA, _inputB, _output );
}

Node_Multiply* Node_Context::createNodeMultiply(Node_Number* _inputA, Node_Number* _inputB, Node_Number* _output)
{
	return (Node_Multiply*)this->createNodeBinaryOperation('*', _inputA, _inputB, _output );
}

Node_Divide* Node_Context::createNodeDivide(Node_Number* _inputA, Node_Number* _inputB, Node_Number* _output)
{
	return (Node_Divide*)this->createNodeBinaryOperation('/', _inputA, _inputB, _output );
}

Node_Assign* Node_Context::createNodeAssign(Node_Number* _inputA, Node_Number* _inputB, Node_Number* _output)
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
								Node_Number* _leftInput, 
								Node_Number* _rightInput, 
								Node_Number* _output)
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
	buildExecutionTreeAndEvaluateRec(currentTokenIndex, result, nullptr);
	//printf("Node_Lexer::buildExecutionTreeAndEvaluate() - DONE !\n");
	cout << "Result: " << result->getValue() << endl;
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
		printf("The only syntax accepted is \"number\", \"operator\", \"number\", etc... \n");
		success = false;
	}

	if(!success)
		printf("Node_Lexer::isSyntaxValid() - FAIL...\n");

	return success;
}

void Node_Lexer::tokenize()
{
	//printf("Node_Lexer::tokenize() - START\n");
	/* get expression chars */
	std::string chars = expression->getValue();

	/* prepare allowed chars */
	string numbers 		= "0123456789.";
	string letters		= "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
	string operators 	= "+-*/=";

	for(auto it = chars.begin(); it < chars.end(); ++it)
	{

		 /* Search for a number */
		/////////////////////////

				if( numbers.find(*it) != string::npos )
		{

			auto itStart = it;
			while(	it != chars.end() && 
					numbers.find(*it) != string::npos)
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

		}else 	if( letters.find(*it) != string::npos)
		{
			auto itStart = it;
			while(	it != chars.end() && 
					letters.find(*it) != string::npos)
			{
				++it;
			}
			--it;

			std::string str = chars.substr(itStart - chars.begin(), it - itStart + 1);
			addToken("symbol", str);

		 /* Search for an operator */
		////////////////////////////
			
		}else 	if(operators.find(*it) != string::npos)
		{
			std::string str = chars.substr(it - chars.begin(), 1);
			addToken("operator", str);
		}		
	}
	//printf("Node_Lexer::tokenize() - DONE !\n");
}

void Node_Lexer::addToken(string _category, string _string)
{
	Token t(_category, _string);
	//printf("Node_Lexer::addToken() - %-10s => \"%s\" \n", ("\"" + _category + "\"").c_str(), _string.c_str() );
	tokens.push_back(t);
}