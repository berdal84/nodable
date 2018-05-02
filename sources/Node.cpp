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