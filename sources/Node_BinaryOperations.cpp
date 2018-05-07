#include "Node_BinaryOperations.h"
#include "Log.h"		// for LOG_DBG(...)
#include "Node_Value.h"
#include "Node_Variable.h"

using namespace Nodable;

 // Node_BinaryOperation :
//////////////////////////

Node_Value* Node_BinaryOperation::getLeftInputValue()const
{
	return dynamic_cast<Node_Value*>(getInput("left")->getValueAsNode());
}


Node_Value* Node_BinaryOperation::getRightInputValue()const
{
	return dynamic_cast<Node_Value*>(getInput("right")->getValueAsNode());
}

Node_Value* Node_BinaryOperation::getOutputValue()const
{
	return dynamic_cast<Node_Value*>(getOutput()->getValueAsNode());
}

/* Precendence for binary operators */
bool Node_BinaryOperation::NeedsToBeEvaluatedFirst(std::string op, std::string nextOp)
{
	if (op == "=" && nextOp == "=") return false;	
	if (op == "=" && nextOp == "-") return false;	
	if (op == "=" && nextOp == "+") return false;	
	if (op == "=" && nextOp == "*") return false;	
	if (op == "=" && nextOp == "/") return false;

	if (op == "+" && nextOp == "=") return false;
	if (op == "+" && nextOp == "-") return true;	
	if (op == "+" && nextOp == "+") return true;	
	if (op == "+" && nextOp == "*") return false;	
	if (op == "+" && nextOp == "/") return false;

	if (op == "-" && nextOp == "=") return false;
	if (op == "-" && nextOp == "-") return true;	
	if (op == "-" && nextOp == "+") return true;	
	if (op == "-" && nextOp == "*") return false;	
	if (op == "-" && nextOp == "/") return false;

	if (op == "*" && nextOp == "=") return false;
	if (op == "*" && nextOp == "-") return true;	
	if (op == "*" && nextOp == "+") return true;	
	if (op == "*" && nextOp == "*") return true;	
	if (op == "*" && nextOp == "/") return true;

	if (op == "/" && nextOp == "=") return false;
	if (op == "/" && nextOp == "-") return true;	
	if (op == "/" && nextOp == "+") return true;	
	if (op == "/" && nextOp == "*") return true;	
	if (op == "/" && nextOp == "/") return true;

	return true;
}

 // Node_Add :
//////////////

void Node_Add::evaluate()
{
	double result = this->getLeftInputValue()->getValueAsNumber() + this->getRightInputValue()->getValueAsNumber();
	LOG_MSG("%s + %s = %f\n", this->getLeftInputValue()->getLabel(), this->getRightInputValue()->getLabel(), result);
	this->getOutputValue()->setValue(result);
}

 // Node_Substract :
///////////////////////

void Node_Substract::evaluate()
{
	double result = this->getLeftInputValue()->getValueAsNumber() - this->getRightInputValue()->getValueAsNumber();
	LOG_MSG("%s - %s = %f\n", this->getLeftInputValue()->getLabel(), this->getRightInputValue()->getLabel(), result);
	this->getOutputValue()->setValue(result);
}

 // Node_Divide :
///////////////////////

void Node_Divide::evaluate()
{
	double result = this->getLeftInputValue()->getValueAsNumber() / this->getRightInputValue()->getValueAsNumber();
	LOG_MSG("%s / %s = %f\n", this->getLeftInputValue()->getLabel(), this->getRightInputValue()->getLabel(), result);
	this->getOutputValue()->setValue(result);
}

 // Node_Multiply :
///////////////////////

void Node_Multiply::evaluate()
{
	double result = this->getLeftInputValue()->getValueAsNumber() * this->getRightInputValue()->getValueAsNumber();
	LOG_MSG("%s * %s = %f\n", this->getLeftInputValue()->getLabel(), this->getRightInputValue()->getLabel(), result);
	this->getOutputValue()->setValue(result);
}

 // Node_Assign :
///////////////////////

void Node_Assign::evaluate()
{
	

	auto result = this->getRightInputValue()->getValueAsNumber();
	this->getLeftInputValue()->setValue(result);
	this->getOutputValue()   ->setValue(result);

	LOG_MSG("%s = %s (result %s)\n", 	this->getLeftInputValue()->getLabel(),
										this->getRightInputValue()->getLabel(),
										this->getOutputValue()->getValueAsString().c_str());	
}