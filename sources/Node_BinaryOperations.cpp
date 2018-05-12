#include "Node_BinaryOperations.h"
#include "Log.h"		// for LOG_DBG(...)
#include "Node_Value.h"
#include "Node_Variable.h"
#include "Node_Value.h"

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

bool Node_Add::evaluate()
{

	switch(getMember("left").getType())
	{
		case Type_String:
		{
			auto result = getMember("left").getValueAsString() + getMember("right").getValueAsString();
			setMember("result", result);
			break;
		}

		default:
		case Type_Number:
		{
			auto result = getMember("left").getValueAsNumber() + getMember("right").getValueAsNumber();
			setMember("result", result);
			break;
		}	
	}
		
	LOG_MSG("%s + %s = %f\n", getMember("left").getValueAsString().c_str(), 
                                  getMember("right").getValueAsString().c_str(),
                                  getMember("result").getValueAsString().c_str());
	return true;
}

 // Node_Substract :
///////////////////////

bool Node_Substract::evaluate()
{

	double result = getMember("left").getValueAsNumber() - getMember("right").getValueAsNumber();
	setMember("result", result);

	LOG_MSG("%s - %s = %f\n", getMember("left").getValueAsString().c_str(), 
                              getMember("right").getValueAsString().c_str(),
                              getMember("result").getValueAsString().c_str());
	return true;
}

 // Node_Divide :
///////////////////////

bool Node_Divide::evaluate()
{
	if (getMember("right").getValueAsNumber() != 0.0f)
	{
		double result = getMember("left").getValueAsNumber() / getMember("right").getValueAsNumber();
		setMember("result", result);
		
		LOG_MSG("%s / %s = %f\n", getMember("left").getValueAsString().c_str(), 
                                  getMember("right").getValueAsString().c_str(),
                                  getMember("result").getValueAsString().c_str());
	}
	return true;
}

 // Node_Multiply :
///////////////////////

bool Node_Multiply::evaluate()
{
	double result = getMember("left").getValueAsNumber() * getMember("right").getValueAsNumber();
	setMember("result", result);
	
	LOG_MSG("%s * %s = %f\n", getMember("left").getValueAsString().c_str(), 
                              getMember("right").getValueAsString().c_str(),
                              getMember("result").getValueAsString().c_str());
	return true;
}

 // Node_Assign :
///////////////////////

bool Node_Assign::evaluate()
{
	switch (getMember("right").getType())
	{
		case Type_Number:
		{
			auto result = getMember("right").getValueAsNumber();
			setMember("result", result);
			setMember("left", result);
			break;
		}
		case Type_String:
		{
			auto result = getMember("right").getValueAsString().c_str();
			setMember("result", result);
			setMember("left", result);
			break;
		}
		default:
		{
			auto result = getMember("right").getValueAsNumber();
			setMember("result", result);
			setMember("left", result);
			break;
		}
	}
	LOG_MSG("%s = %s (result %s)\n", 	getMember("left").getValueAsString().c_str(),
										getMember("right").getValueAsString().c_str(),
										getMember("result").getValueAsString().c_str());	
	return true;
}