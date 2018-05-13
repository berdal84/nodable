#include "Node_BinaryOperations.h"
#include "Log.h"		// for LOG_DBG(...)
#include "Value.h"
#include "Node_Variable.h"
#include "Value.h"

using namespace Nodable;

 // Node_BinaryOperation :
//////////////////////////

Node_BinaryOperation::Node_BinaryOperation()
{
	addMember("left");
	addMember("right");
	addMember("result");
}

bool Node_BinaryOperation::couldBeEvaluated()
{
	return true;
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

bool Node_Add::eval()
{
	switch(getMember("left")->getType())
	{
		case Type_String:
		{
			auto result = getMember("left")->getValueAsString() + getMember("right")->getValueAsString();
			getMember("result")->setValue(result);
			break;
		}

		default:
		case Type_Number:
		{
			auto result = getMember("left")->getValueAsNumber() + getMember("right")->getValueAsNumber();
			getMember("result")->setValue(result);
			break;
		}	
	}
		
	LOG_MSG("%s + %s = %f\n", getMember("left")->getValueAsString().c_str(), 
                                  getMember("right")->getValueAsString().c_str(),
                                  getMember("result")->getValueAsString().c_str());
	return true;
}

 // Node_Substract :
///////////////////////

bool Node_Substract::eval()
{
	double result = getMember("left")->getValueAsNumber() - getMember("right")->getValueAsNumber();
	getMember("result")->setValue(result);

	LOG_MSG("%s - %s = %f\n", getMember("left")->getValueAsString().c_str(), 
                              getMember("right")->getValueAsString().c_str(),
                              getMember("result")->getValueAsString().c_str());
	return true;
}

 // Node_Divide :
///////////////////////

bool Node_Divide::eval()
{
	if (getMember("right")->getValueAsNumber() != 0.0f)
	{
		double result = getMember("left")->getValueAsNumber() / getMember("right")->getValueAsNumber();
		getMember("result")->setValue(result);
		
		LOG_MSG("%s / %s = %f\n", getMember("left")->getValueAsString().c_str(), 
                                  getMember("right")->getValueAsString().c_str(),
                                  getMember("result")->getValueAsString().c_str());
	}
	return true;
}

 // Node_Multiply :
///////////////////////

bool Node_Multiply::eval()
{
	double result = getMember("left")->getValueAsNumber() * getMember("right")->getValueAsNumber();
	getMember("result")->setValue(result);
	
	LOG_MSG("%s * %s = %f\n", getMember("left")->getValueAsString().c_str(), 
                              getMember("right")->getValueAsString().c_str(),
                              getMember("result")->getValueAsString().c_str());
	setDirty(false);

	return true;
}

 // Node_Assign :
///////////////////////

bool Node_Assign::eval()
{
	switch (getMember("right")->getType())
	{
		case Type_Number:
		{
			auto result = getMember("right")->getValueAsNumber();
			getMember("result")->setValue(result);
			getMember("left")->setValue(result);
			break;
		}
		case Type_String:
		{
			auto result = getMember("right")->getValueAsString().c_str();
			getMember("result")->setValue(result);
			getMember("left")->setValue(result);
			break;
		}
		default:
		{
			auto result = getMember("right")->getValueAsNumber();
			getMember("result")->setValue(result);
			getMember("left")->setValue(result);
			break;
		}
	}
	LOG_MSG("%s = %s (result %s)\n", 	getMember("left")->getValueAsString().c_str(),
										getMember("right")->getValueAsString().c_str(),
										getMember("result")->getValueAsString().c_str());	
	return true;
}