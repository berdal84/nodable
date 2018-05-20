#include "BinaryOperationComponents.h"
#include "Log.h"		// for LOG_DBG(...)
#include "Value.h"
#include "Node_Variable.h"
#include "Value.h"

using namespace Nodable;

 // BinaryOperationComponent :
//////////////////////////

/* Precendence for binary operators */
bool BinaryOperationComponent::NeedsToBeEvaluatedFirst(std::string op, std::string nextOp)
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

void Add::update()
{
	switch(left->getType())
	{
		case Type_String:
		{
			auto sum = left->getValueAsString() + right->getValueAsString();
			result->setValue(sum);
			break;
		}

		default:
		case Type_Number:
		{
			auto sum = left->getValueAsNumber() + right->getValueAsNumber();
			result->setValue(sum);
			break;
		}	
	}
		
	LOG_MSG("%s + %s = %f\n", left->getValueAsString().c_str(), 
                              right->getValueAsString().c_str(),
                              result->getValueAsString().c_str());
}

 // Node_Substract :
///////////////////////

void Substract::update()
{
	double sub = left->getValueAsNumber() - right->getValueAsNumber();
	result->setValue(sub);

	LOG_MSG("%s - %s = %f\n", left->getValueAsString().c_str(), 
                              right->getValueAsString().c_str(),
                              result->getValueAsString().c_str());
}

 // Node_Divide :
///////////////////////

void Divide::update()
{
	if (right->getValueAsNumber() != 0.0f)
	{
		double div = left->getValueAsNumber() / right->getValueAsNumber();
		result->setValue(div);
		
		LOG_MSG("%s / %s = %f\n", left->getValueAsString().c_str(), 
                                  right->getValueAsString().c_str(),
                                  result->getValueAsString().c_str());
	}
}

 // Node_Multiply :
///////////////////////

void Multiply::update()
{
	double mul = left->getValueAsNumber() * right->getValueAsNumber();
	result->setValue(mul);
	
	LOG_MSG("%s * %s = %f\n", left->getValueAsString().c_str(), 
                              right->getValueAsString().c_str(),
                              result->getValueAsString().c_str());
}

 // Node_Assign :
///////////////////////

void Assign::update()
{
	switch (right->getType())
	{
		case Type_Number:
		{
			auto v = right->getValueAsNumber();
			result->setValue(v);
			left->setValue(v);
			break;
		}
		case Type_String:
		{
			auto v = right->getValueAsString().c_str();
			result->setValue(v);
			left->setValue(v);
			break;
		}
		default:
		{
			auto v = right->getValueAsNumber();
			result->setValue(v);
			left->setValue(v);
			break;
		}
	}
	LOG_MSG("%s = %s (result %s)\n", 	left->getValueAsString().c_str(),
										right->getValueAsString().c_str(),
										result->getValueAsString().c_str());	
}