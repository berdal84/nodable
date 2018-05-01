#include "Node.h"
#include "Log.h"		// for LOG_DBG(...)
#include <algorithm>    // std::find_if
#include <cstring>      // for strcmp
#include "Node_Value.h"
#include "Node_Number.h"
#include "Node_String.h"
#include "Node_Lexer.h"

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
	LOG_MSG("%f + %f = %f\n", this->getLeftInput()->asNumber()->getValue(), this->getRightInput()->asNumber()->getValue(), result);
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
	LOG_MSG("%f - %f = %f\n", this->getLeftInput()->asNumber()->getValue(), this->getRightInput()->asNumber()->getValue(), result);
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
	LOG_MSG("%f / %f = %f\n", this->getLeftInput()->asNumber()->getValue(), this->getRightInput()->asNumber()->getValue(), result);
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
	LOG_MSG("%f * %f = %f\n", this->getLeftInput()->asNumber()->getValue(), this->getRightInput()->asNumber()->getValue(), result);
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
		LOG_DBG("unable to assign with two different value types\n");
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
	LOG_DBG("New Node_Symbol : %s\n", _name);
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
	LOG_DBG("A new context named '%s' has been created.\n", _name);
}


void Node_Context::addNode(Node* _node)
{
	/* Add the node to the node vector list*/
	this->nodes.push_back(_node);

	/* Set the node's context to this */
	_node->setContext(this);

	LOG_DBG("A node has been added to the context '%s'\n", this->getName());
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
		LOG_DBG("FOUND !\n");
		return *it;
	}
	LOG_DBG("NOT found...\n");
	return nullptr;
}

Node_Symbol* Node_Context::createNodeSymbol(const char* _name, Node_Value* _value)
{
	Node_Symbol* node = new Node_Symbol(_name, _value);
	addNode(node);
	this->symbols.push_back(node);
	return node;
}

Node_Number*          Node_Context::createNodeNumber(int _value)
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

Node_String*          Node_Context::createNodeString(const char* _value)
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