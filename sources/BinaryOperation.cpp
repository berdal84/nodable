#include "BinaryOperation.h"
#include "Log.h"		// for LOG_DEBUG(...)
#include "Member.h"
#include "Variable.h"
#include "Language.h"

using namespace Nodable;

 // BinaryOperationComponent :
//////////////////////////

void BinaryOperationComponent::updateResultSourceExpression()const
{
	/*
		Labmda funtion to check if parentheses are needed for the expression of the inputMember speficied as parameter.
	*/
	auto needParentheses = [&](Member * _input)->bool
	{
		if (_input != nullptr )
		{
			auto entity = _input->getOwner()->as<Entity>();

			if (entity->hasComponent("operation"))
			{

				if (auto leftOperationComponent = entity->getComponent<BinaryOperationComponent>("operation"))
				{
					auto leftOperatorString = leftOperationComponent->getOperatorAsString();

					if (leftOperatorString == this->operatorAsString)
						return false;

					if (NeedsToBeEvaluatedFirst(this->operatorAsString, leftOperatorString))
						return true;
				}
			}
		}
		return false;
	};

	std::string expr;

	// Left part of the expression
	bool leftExpressionNeedsParentheses  = needParentheses(this->left->getInputMember());
	if (leftExpressionNeedsParentheses) expr.append("( ");
	expr.append( this->left->getSourceExpression() );
	if (leftExpressionNeedsParentheses) expr.append(" )");

	// Operator
	expr.append( " " );
	expr.append( this->operatorAsString );
	expr.append( " " );

	// Righ part of the expression
	bool rightExpressionNeedsParentheses = needParentheses(this->right->getInputMember());
	if (rightExpressionNeedsParentheses) expr.append("( ");
	expr.append(this->right->getSourceExpression());
	if (rightExpressionNeedsParentheses) expr.append(" )");

	// Apply the new string to the result's source expression.
	this->result->setSourceExpression(expr.c_str());
}

/* Precendence for binary operators */
bool BinaryOperationComponent::NeedsToBeEvaluatedFirst(std::string op, std::string nextOp)
{
	auto language = Language::NODABLE;
	const bool isHigher = language->getOperatorPrecedence(op) >= language->getOperatorPrecedence(nextOp);

	return isHigher;
	
}

 // Node_Add :
//////////////

bool Add::update()
{
	switch(left->getType())
	{
		case Type_String:
		{
			auto sum = left->getValueAsString() + right->getValueAsString();
			result->setValue(sum);
			break;
		}

		case Type_Boolean:
		{
			auto sum = left->getValueAsBoolean() || right->getValueAsBoolean();
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
	
	updateResultSourceExpression();

	return true;
}

 // Node_Substract :
///////////////////////

bool Substract::update()
{
	double sub = left->getValueAsNumber() - right->getValueAsNumber();
	result->setValue(sub);
	
	updateResultSourceExpression();

	return true;
}

 // Node_Divide :
///////////////////////

bool Divide::update()
{
	if (right->getValueAsNumber() != 0.0f)
	{
		auto div = left->getValueAsNumber() / right->getValueAsNumber();
		result->setValue(div);
	}

	updateResultSourceExpression();

	return true;
}

 // Node_Multiply :
///////////////////////

bool Multiply::update()
{
	switch(left->getType())
	{
		case Type_Boolean:
		{
			auto mul = left->getValueAsBoolean() && right->getValueAsBoolean();
			result->setValue(mul);
			break;
		}	

		default:
		{
			auto mul = left->getValueAsNumber() * right->getValueAsNumber();
			result->setValue(mul);
			break;
		}
	}
	
	updateResultSourceExpression();

	return true;
}

 // Node_Assign :
///////////////////////

bool Assign::update()
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
	
	updateResultSourceExpression();

	return true;
}