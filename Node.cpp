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

Node_BinaryOperation* Node_BinaryOperation::Create(   const char _operator, 
								Node_Number* _leftInput, 
								Node_Number* _rightInput, 
								Node_Number* _output)
{
	if ( _operator == '+')
		return new Node_Add(_leftInput, _rightInput, _output);
	if ( _operator == '-')
		return new Node_Substract(_leftInput, _rightInput, _output);
	if ( _operator == '/')
		return new Node_Divide(_leftInput, _rightInput, _output);
	if ( _operator == '*')
		return new Node_Multiply(_leftInput, _rightInput, _output);

	return nullptr;
}

/* Precendence for binary operators */
bool Node_BinaryOperation::NeedsToBeEvaluatedFirst(const char op, const char nextOp)
{
	if (op == '+' & nextOp == '-') return true;	
	if (op == '+' & nextOp == '+') return true;	
	if (op == '+' & nextOp == '*') return false;	
	if (op == '+' & nextOp == '/') return false;

	if (op == '-' & nextOp == '-') return true;	
	if (op == '-' & nextOp == '+') return true;	
	if (op == '-' & nextOp == '*') return false;	
	if (op == '-' & nextOp == '/') return false;

	if (op == '*' & nextOp == '-') return true;	
	if (op == '*' & nextOp == '+') return true;	
	if (op == '*' & nextOp == '*') return true;	
	if (op == '*' & nextOp == '/') return true;

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
	cout <<  "Node_Add:evaluate(): " <<  this->getLeftInput()->getValue() << " + " << this->getRightInput()->getValue() << " = " << result << endl;
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
	cout <<  "Node_Substract:evaluate(): " <<  this->getLeftInput()->getValue() << " - " << this->getRightInput()->getValue() << " = " << result << endl;
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
	cout <<  "Node_Divide:evaluate(): " <<  this->getLeftInput()->getValue() << " / " << this->getRightInput()->getValue() << " = " << result << endl;
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
	cout <<  "Node_Multiply:evaluate(): " <<  this->getLeftInput()->getValue() << " * " << this->getRightInput()->getValue() << " = " << result << endl;
	this->getOutput()->setValue(result);
}
 // Node_Tag :
//////////////

Node_Tag::Node_Tag(Node_Context* _context, const char* _name, Node* _value):
	name(_name),
	value(_value),
	context(_context)	
{
	//cout << "New Node_Tag : " << _name << endl;
	_context->add(this);
}

Node_Tag::~Node_Tag()
{

}

const char* Node_Tag::getName()const
{
	return name.c_str();
}

Node* Node_Tag::getValue()const
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

void Node_Context::add(Node_Tag* _node)
{
	tags.push_back(_node);
}

Node_Tag* Node_Context::find(const char* _name)
{
	printf("Searching node with name '%s' in context named '%s' : ", _name, this->name.c_str());

	auto findFunction = [_name](const Node_Tag* _node ) -> bool
	{
		return strcmp(_node->getName(), _name) == 0;
	};

	auto it = std::find_if(tags.begin(), tags.end(), findFunction);
	if (it != tags.end()){
		cout << "found." << endl;
		return *it;
	}
	cout << "NOT found !" << endl;
	return nullptr;
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

void Node_Lexer::buildExecutionTreeAndEvaluateRec(size_t _tokenIndex, Node_Number* _finalRes, Node_Number* _prevRes)
{

	Node_Number* 	      left;
	Node_Number*         right;
	Node_BinaryOperation* operation;

	//printf("Token evaluated : %lu.\n", _tokenIndex);

	// If a previous result is set, it mean we have already calculated the left part
	// So we use it as left operand

	if ( _prevRes != nullptr ){
		left = _prevRes;

	// Else, we parse the left operand
	}else{
		left = new Node_Number(tokens[_tokenIndex].second.c_str());
	}
	

	size_t tokenLeft = tokens.size() - _tokenIndex;
	/* number */
	if ( tokenLeft == 1)
	{
		_finalRes->setValue(left->getValue());

	/* number, op, expr */
	}else if  (tokenLeft == 3){
		const char op = *tokens[_tokenIndex+1].second.c_str();
		right 	= new Node_Number(tokens[_tokenIndex+2].second.c_str());
		buildExecutionTreeAndEvaluateRec(_tokenIndex+2, right, nullptr);		
		operation = Node_BinaryOperation::Create(op, left, right, _finalRes);
		operation->evaluate();

	/* number, op, number, op, expr */
	}else if  (tokenLeft >= 4)
	{	
		const char op = *tokens[_tokenIndex+1].second.c_str();	
		right 	= new Node_Number(tokens[_tokenIndex+2].second.c_str());
		const char nextOp = *tokens[_tokenIndex+3].second.c_str();	

		/* if currOperator is more important than nextOperator
		   we perform the first operation and send the result as left operand to the next expression */

		bool evaluateNow = Node_BinaryOperation::NeedsToBeEvaluatedFirst(op, nextOp);

		if ( evaluateNow ){
			// Perform the operation on the left
			Node_Number* 	result 	= new Node_Number();			
			operation 		        = Node_BinaryOperation::Create(op, left, right, result);
			operation->evaluate();
			// Pass the result and build the next operations
			buildExecutionTreeAndEvaluateRec(_tokenIndex+2, _finalRes, result);	

		/* Else, we evaluate the next expression and then perform the operation with 
		the result of the next expresssion as right operand */
		}else{
			buildExecutionTreeAndEvaluateRec(_tokenIndex+2, right, nullptr);		
			operation = Node_BinaryOperation::Create( op, left, right, _finalRes);	
			operation->evaluate();
		}
	}
}


void Node_Lexer::buildExecutionTreeAndEvaluate()
{
	//printf("Node_Lexer::buildExecutionTreeAndEvaluate() - START\n");
	auto currentTokenIndex = 0;
	Node_Number* result = new Node_Number();	
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
			if ( tokens[i].first != "number")
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
	string operators 	= "+-*/";

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